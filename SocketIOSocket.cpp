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


SocketIOSocket::SocketIOSocket(SocketIOManager* io, const std::string& nsp, const Opts& opts)
{
  _io = io;
  _nsp = nsp;
  _ids = 0;
  _acks.clear();
  _receiveBuffer.clear();
  _sendBuffer.clear();
  _connected = false;
  _disconnected = true;
  if (opts && opts.query) {
    _query = opts.query;
  }
  if (_io->autoConnect) this.open();
}

void SocketIOSocket::subEvents()
{
  if (_subs) return;

  _subs.push_back(on(_io, "open", std::bind(SocketIOSocket::onopen, this)));
  _subs.push_back(on(_io, "packet", std::bind(SocketIOSocket::onpacket, this)));
  _subs.push_back(on(_io, "close", std::bind(SocketIOSocket::onclose, this)));
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

void SocketIOSocket::send(const Args& args)
{
  emit("message", args);
}

void SocketIOSocket::emit(const std::string& eventName, const Args& args)
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
  packet.data = args;

  packet.options = {};
  packet.options.compress = _compress;

  // event ack callback
  if ('function' == typeof args[args.length - 1]) {
    debug('emitting packet with ack id %d', _ids);
    _acks[_ids] = args.pop_back();
    packet.id = _ids++;
  }

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

void SocketIOSocket::onopen()
{
  debug('transport is open - connecting');

  // write connect packet if necessary
  if ("/" != _nsp) {
    SocketIOPacket packet;
    packet.setType(SocketIOPacket::Type::CONNECT);

    if (!_query.empty()) {
      packet.setQuery(_query);
      sendPacket(packet);
    } else {
      sendPacket(packet);
    }
  }
}

void SocketIOSocket::onclose(const std::string& reason)
{
  debug('close (%s)', reason);
  _connected = false;
  _disconnected = true;
  _id.clear();
  emit('disconnect', reason);
}

void SocketIOSocket::onpacket(const SocketIOPacket& packet)
{
  if (packet.getNsp() != _nsp) return;

  switch (packet.getType()) {
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
  var args = packet.data || [];
  debug('emitting event %j', args);

  if (packet.getId() != -1) {
    debug('attaching ack callback to event');
    args.push_back(ack(packet.getId()));
  }

  if (_connected) {
    emit(args);
  } else {
    _receiveBuffer.push_back(args);
  }
}

AckCallback SocketIOSocket::ack(int id)
{
  std::shared_ptr<bool> sent = std::make_shared<bool>(false);
  return [this, sent](const Data& data) {
    // prevent double callbacks
    if (*sent) return;
    *sent = true;
    debug('sending ack %j', data);

    SocketIOPacket::Type type = hasBin(data) ? SocketIOPacket::Type::BINARY_ACK : SocketIOPacket::Type::ACK;

    SocketIOPacket packet;
    packet.type = type;
    packet.id = id;
    packet.data = data

    sendPacket(packet);
  };
}

void SocketIOSocket::onack(const SocketIOPacket& packet)
{
  auto iter = _acks.find(packet.getId());
  if (iter != _acks.end()) {
    debug('calling ack %s with %j', packet.id, packet.data);
    iter->second(packet.data);
    _acks.erase(packet.getId());
  } else {
    debug('bad ack %s', packet.id);
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

  for (i = 0; i < _sendBuffer.length; i++) {
    sendPacket(_sendBuffer[i]);
  }
  _sendBuffer.clear();
}

void SocketIOSocket::ondisconnect()
{
  debug('server disconnect (%s)', _nsp);
  destroy();
  onclose('io server disconnect');
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
    debug('performing disconnect (%s)', _nsp);
    sendPacket({ type: SocketIOPacket::Type::DISCONNECT });
  }

  // remove socket from pool
  destroy();

  if (_connected) {
    // fire events
    onclose('io client disconnect');
  }
}

void SocketIOSocket::setCompress(bool compress)
{
    _compress = compress;
}
