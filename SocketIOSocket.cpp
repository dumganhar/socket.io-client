#include "SocketIOSocket.h"
#include "SocketIOManager.h"
#include "IOUtils.h"

#include <assert.h>

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

  _subs.push_back(gon(_io, "open", std::bind(&SocketIOSocket::onopen, this, std::placeholders::_1)));
  _subs.push_back(gon(_io, "packet", std::bind(&SocketIOSocket::onpacket, this, std::placeholders::_1)));
  _subs.push_back(gon(_io, "close", std::bind(&SocketIOSocket::onclose, this, std::placeholders::_1)));
}

// connect
void SocketIOSocket::open()
{
  if (_connected) return;

  subEvents();
  _io->connect(nullptr, Opts()); // ensure open
  if (ReadyState::OPENED == _io->getReadyState())
      onopen(Value::NONE);
  emit("connecting");
}

void SocketIOSocket::send(const Value& args)
{
  emit("message", args);
}

void SocketIOSocket::emit(const Value& args)
{
    assert(args.getType() == Value::Type::ARRAY);

    ValueArray arguments = args.asArray();
    if (arguments.empty()) return;

    const Value& event = arguments.at(0);
    assert(event.getType() == Value::Type::STRING);

    const std::string& eventName = event.asString();

    if (std::find(__events.begin(), __events.end(), eventName) != __events.end())
    {
        Emitter::emit(eventName, args);
        return;
    }

    SocketIOPacket::Type parserType = SocketIOPacket::Type::EVENT; // default
    if (args.hasBin()) {
        parserType = SocketIOPacket::Type::BINARY_EVENT;
    } // binary

    SocketIOPacket packet;
    packet.type = parserType;
    packet.options["compress"] = _compress;

    // event ack callback
    if (arguments[arguments.size() - 1].getType() == Value::Type::FUNCTION) {
        debug("emitting packet with ack id %d", _ids);
        _acks[_ids] = arguments[arguments.size() - 1].asFunction();
        packet.id = _ids++;
        arguments.pop_back();
    }

    packet.data = arguments;

    if (_connected) {
        sendPacket(packet);
    } else {
        _sendBuffer.push_back(packet);
    }
}

void SocketIOSocket::emit(const std::string& eventName, const Value& args)
{
    ValueArray arguments = Value::concat(eventName, args);
    emit(arguments);
}

void SocketIOSocket::sendPacket(const SocketIOPacket& packet)
{
  const_cast<SocketIOPacket&>(packet).nsp = _nsp;
  _io->sendPacket(const_cast<SocketIOPacket&>(packet));
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
  debug("close (%s)", reason.asString().c_str());
  _connected = false;
  _disconnected = true;
  _id.clear();
  emit("disconnect", reason);
}

void SocketIOSocket::onpacket(const Value& v)
{
  const SocketIOPacket& packet = v.asSocketIOPacket();
  if (packet.nsp != _nsp) return;

  switch (packet.type) {
    case SocketIOPacket::Type::CONNECT:
          onconnect();
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

void SocketIOSocket::onevent(const SocketIOPacket& packet)
{
    const Value& args = packet.data;
    debug("emitting event %s", args.toString().c_str());

    ValueArray arguments;
  if (packet.id != -1) {
    debug("attaching ack callback to event");
    arguments = Value::concat(args, ack(packet.id));
  }

  if (_connected) {
    emit(args);
  } else {
    _receiveBuffer.push_back(args);
  }
}

ValueFunction SocketIOSocket::ack(int id)
{
  std::shared_ptr<bool> sent = std::make_shared<bool>(false);
  return [this, sent, id](const Value& data) {
    // prevent double callbacks
    if (*sent) return;
    *sent = true;
    debug("sending ack %s", data.toString().c_str());

    SocketIOPacket::Type type = data.hasBin() ? SocketIOPacket::Type::BINARY_ACK : SocketIOPacket::Type::ACK;

    SocketIOPacket packet;
    packet.type = type;
    packet.id = id;
    packet.data = data;

    sendPacket(packet);
  };
}

void SocketIOSocket::onack(const SocketIOPacket& packet)
{
  auto iter = _acks.find(packet.id);
  if (iter != _acks.end()) {
    debug("calling ack %d with %s", packet.id, packet.data.toString().c_str());
    iter->second(packet.data);
    _acks.erase(packet.id);
  } else {
    debug("bad ack %d", packet.id);
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

  for (size_t i = 0; i < _sendBuffer.size(); i++) {
    sendPacket(_sendBuffer[i]);
  }
  _sendBuffer.clear();
}

void SocketIOSocket::ondisconnect()
{
  debug("server disconnect (%s)", _nsp.c_str());
  destroy();
  onclose("io server disconnect");
}

void SocketIOSocket::destroy()
{
  if (!_subs.empty()) {
    // clean subscriptions to avoid reconnections
    for (auto& sub : _subs) {
      sub.destroy();
    }
    _subs.clear();
  }

  _io->destroySocket(shared_from_this());
}

// disconnect
void SocketIOSocket::close()
{
  if (_connected) {
    debug("performing disconnect (%s)", _nsp.c_str());
      SocketIOPacket packet;
      packet.type = SocketIOPacket::Type::DISCONNECT;
      sendPacket(packet);
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
