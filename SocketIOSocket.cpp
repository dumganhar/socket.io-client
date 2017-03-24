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

  var args = toArray(arguments);
  PacketType parserType = PacketType::EVENT; // default
  if (hasBin(args)) { 
    parserType = PacketType::BINARY_EVENT;
  } // binary
  Packet packet;
  packet.type = parserType;
  packet.data = args;

  packet.options = {};
  packet.options.compress = _compress;

  // event ack callback
  if ('function' == typeof args[args.length - 1]) {
    debug('emitting packet with ack id %d', _ids);
    _acks[_ids] = args.pop();
    packet.id = _ids++;
  }

  if (_connected) {
    sendPacket(packet);
  } else {
    _sendBuffer.push_back(packet);
  }
}

void SocketIOSocket::sendPacket(const Packet& packet)
{
  packet.nsp = _nsp;
  _io->sendPacket(packet);
}

void SocketIOSocket::onopen()
{
  debug('transport is open - connecting');

  // write connect packet if necessary
  if ("/" != _nsp) {
    if (!_query.empty()) {
      sendPacket({type: PacketType::CONNECT, query: _query});
    } else {
      sendPacket({type: PacketType::CONNECT});
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

void SocketIOSocket::onpacket(const Packet& packet)
{
  if (packet.nsp != _nsp) return;

  switch (packet.type) {
    case PacketType::CONNECT:
      onconnect();
      break;

    case PacketType::EVENT:
      onevent(packet);
      break;

    case PacketType::BINARY_EVENT:
      onevent(packet);
      break;

    case PacketType::ACK:
      onack(packet);
      break;

    case PacketType::BINARY_ACK:
      onack(packet);
      break;

    case PacketType::DISCONNECT:
      ondisconnect();
      break;

    case PacketType::ERROR:
      emit("error", packet.data);
      break;
  }
}

void SocketIOSocket::onevent(const Packet& packet)
{
  var args = packet.data || [];
  debug('emitting event %j', args);

  if (null != packet.id) {
    debug('attaching ack callback to event');
    args.push(ack(packet.id));
  }

  if (_connected) {
    emit.apply(this, args);
  } else {
    _receiveBuffer.push_back(args);
  }
}

void SocketIOSocket::ack(int id)
{
  std::shared_ptr<bool> sent = std::make_shared<bool>(false);
  return [this, sent]() {
    // prevent double callbacks
    if (*sent) return;
    *sent = true;
    var args = toArray(arguments);
    debug('sending ack %j', args);

    var type = hasBin(args) ? PacketType::BINARY_ACK : PacketType::ACK;
    sendPacket({
      type: type,
      id: id,
      data: args
    });
  };
}

void SocketIOSocket::onack(const Packet& packet)
{
  var ack = _acks[packet.id];
  if ('function' == typeof ack) {
    debug('calling ack %s with %j', packet.id, packet.data);
    ack.apply(this, packet.data);
    delete _acks[packet.id];
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

  }
  size_t i;
  for (i = 0; i < _receiveBuffer.size(); i++) {
    emit.apply(this, _receiveBuffer[i]);
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
    sendPacket({ type: PacketType::DISCONNECT });
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
