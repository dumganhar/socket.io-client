#include "SocketIOSocket.h"
#include "SocketIOManager.h"
#include "IOUtils.h"

/**
 * Internal events (blacklisted).
 * These events can't be emitted by the user.
 *
 * @api private
 */

static std::vector<std::string> __events = {
  "connect",
  "connect_error",
  "connect_timeout",
  "connecting",
  "disconnect",
  "error",
  "reconnect",
  "reconnect_attempt",
  "reconnect_failed",
  "reconnect_error",
  "reconnecting",
  "ping",
  "pong"
};


SocketIOSocket::SocketIOSocket(std::shared_ptr<SocketIOManager> io, const std::string& nsp, const Opts& opts)
{
  _io = io;
  _nsp = nsp;
  _ids = 0;
  _acks.clear();
  _receiveBuffer.clear();
  _sendBuffer.clear();
  _connected = false;
  _disconnected = true;
  if (opts.isValid() && !opts.query.empty()) {
    _query = opts.query;
  }
  if (_io->_autoConnect)
      open();
}

void SocketIOSocket::subEvents()
{
  if (_subs.empty()) return;

  _subs.push_back(gon(_io, "open", std::bind(SocketIOSocket::onopen, this), ID()));
  _subs.push_back(gon(_io, "packet", std::bind(SocketIOSocket::onpacket, this), ID()));
  _subs.push_back(gon(_io, "close", std::bind(SocketIOSocket::onclose, this), ID()));
}

// connect
void SocketIOSocket::open()
{
  if (_connected) return;

  subEvents();
  _io->open(); // ensure open
  if (ReadyState::OPEN == _io->getReadyState())
    onopen();
  emit("connecting");
}

void SocketIOSocket::send(Args& args)
{
  emit("message", args);
}

void SocketIOSocket::emit(const Value& args)
{
    //cjh
}

void SocketIOSocket::emit(const std::string& eventName, const Value& args)
{
  if (__events.find(eventName) != __events.end())
  {
    Emitter::emit(eventName, args);
    return;
  }

  SocketIOPacket::Type parserType = SocketIOPacket::Type::EVENT; // default
  if (hasBin(args)) { 
    parserType = SocketIOPacket::Type::BINARY_EVENT;
  } // binary
  SocketIOPacket packet;
  packet.type = parserType;
  packet.options["compress"] = _compress;

  // event ack callback
  if (args[args.size() - 1].isFunction()) {
    debug("emitting packet with ack id %d", _ids);
    _acks[_ids] = args.pop_back();
    packet.id = _ids++;
  }

    ValueArray arguments;
    arguments.push_back(eventName);
    if (args.getType() == Value::Type::ARRAY)
    {
        const ValueArray& arr = args.asArray();
        arguments.reserve(arr.size() + 1);
        arguments.insert(arguments.end(), arr.begin(), arr.end());
    }
    else
    {
        arguments.push_back(args);
    }

  packet.data = arguments;

  if (_connected) {
    sendPacket(packet);
  } else {
    _sendBuffer.push_back(packet);
  }
}

void SocketIOSocket::sendPacket(const SocketIOPacket& packet)
{
  const_cast<SocketIOPacket&>(packet).setNsp(_nsp);
  _io->sendPacket(packet);
}

void SocketIOSocket::onopen(const Value& v)
{
  debug("transport is open - connecting");

  // write connect packet if necessary
  if ("/" != _nsp) {
    SocketIOPacket packet;
    packet.type = SocketIOPacket::Type::CONNECT;

    if (!_query.empty()) {
      packet.query = _query;
      sendPacket(packet);
    } else {
      sendPacket(packet);
    }
  }
}

void SocketIOSocket::onclose(const Value& reason)
{
  debug("close (%s)", reason);
  _connected = false;
  _disconnected = true;
  _id.clear();
  emit("disconnect", reason);
}

void SocketIOSocket::onpacket(const Value& packet)
{
  if (packet.nsp != _nsp) return;

  switch (packet.getType()) {
    case SocketIOPacket::Type::CONNECT:
          onconnect(Value::NONE);
      break;

    case SocketIOPacket::Type::EVENT:
      onevent(packet);
      break;

    case SocketIOPacket::Type::BINARY_EVENT:
      onevent(packet);
      break;

    case SocketIOPacket::Type::ACK:
      onack(packet);
      break;

    case SocketIOPacket::Type::BINARY_ACK:
      onack(packet);
      break;

    case SocketIOPacket::Type::DISCONNECT:
      ondisconnect();
      break;

    case SocketIOPacket::Type::ERROR:
      emit("error", packet.data);
      break;
  }
}

void SocketIOSocket::onevent(const Value& packet)
{
  Value& args = packet.data;
  debug("emitting event %s", args.toString().c_str());

  if (packet.getId() != -1) {
    debug("attaching ack callback to event");
    args.push_back(ack(packet.getId()));
  }

  if (_connected) {
    emit(packet.getEventName(), args);
  } else {
    _receiveBuffer.push_back(args);
  }
}

ValueFunction SocketIOSocket::ack(int id)
{
  std::shared_ptr<bool> sent = std::make_shared<bool>(false);
  return [this, sent](const Value& data) {
    // prevent double callbacks
    if (*sent) return;
    *sent = true;
    debug("sending ack %s", data.toString().c_str());

    SocketIOPacket::Type type = hasBin(data) ? SocketIOPacket::Type::BINARY_ACK : SocketIOPacket::Type::ACK;

    SocketIOPacket packet;
    packet.setType(type);
    packet.setId(id);
    packet.setData(data);

    sendPacket(packet);
  };
}

void SocketIOSocket::onack(const Value& packet)
{
  auto iter = _acks.find(packet.getId());
  if (iter != _acks.end()) {
    debug("calling ack %s with %j", packet.id, packet.data);
    iter->second(packet.data);
    _acks.erase(packet.getId());
  } else {
    debug("bad ack %s", packet.id);
  }
}

void SocketIOSocket::onconnect()
{
  _connected = true;
  _disconnected = false;
  emit("connect");
  emitBuffered();
}

void SocketIOSocket::emitBuffered()
{
  for (auto& receivedBuf : _receiveBuffer)
  {
      emit(receivedBuf);
  }

  _receiveBuffer.clear();

  for (i = 0; i < _sendBuffer.size(); i++) {
    sendPacket(_sendBuffer[i]);
  }
  _sendBuffer.clear();
}

void SocketIOSocket::ondisconnect()
{
  debug("server disconnect (%s)", _nsp);
  destroy();
  onclose("io server disconnect");
}

void SocketIOSocket::destroy()
{
  if (!_subs.empty()) {
    // clean subscriptions to avoid reconnections
    for (auto& sub : _subs) {
      sub->destroy();
    }
    _subs.clear();
  }

  _io->destroySocket(this);
}

// disconnect
void SocketIOSocket::close()
{
  if (_connected) {
    debug("performing disconnect (%s)", _nsp);
    sendPacket({ type: SocketIOPacket::Type::DISCONNECT });
  }

  // remove socket from pool
  destroy();

  if (_connected) {
    // fire events
    onclose("io client disconnect");
  }
}

void SocketIOSocket::setCompress(bool compress)
{
    _compress = compress;
}
