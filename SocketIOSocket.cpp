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
    this.query = opts.query;
  }
  if (_io->autoConnect) this.open();
}

void SocketIOSocket::subEvents()
{
  if (_subs) return;

  _subs.push_back(on(_io, "open", bind(this, 'onopen')));
  _subs.push_back(on(_io, "packet", bind(this, 'onpacket')));
  _subs.push_back(on(_io, "close", bind(this, 'onclose')));
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
  var parserType = parser.EVENT; // default
  if (hasBin(args)) { 
    parserType = parser.BINARY_EVENT;
  } // binary
  var packet = { type: parserType, data: args };

  packet.options = {};
  packet.options.compress = _compress;

  // event ack callback
  if ('function' == typeof args[args.length - 1]) {
    debug('emitting packet with ack id %d', this.ids);
    _acks[this.ids] = args.pop();
    packet.id = this.ids++;
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
    if (this.query) {
      sendPacket({type: parser.CONNECT, query: this.query});
    } else {
      sendPacket({type: parser.CONNECT});
    }
  }
}

void SocketIOSocket::onclose(const std::string& reason)
{
  debug('close (%s)', reason);
  _connected = false;
  _disconnected = true;
  delete this.id;
  this.emit('disconnect', reason);
}

void SocketIOSocket::onpacket(const Packet& packet)
{
  if (packet.nsp != _nsp) return;

  switch (packet.type) {
    case parser.CONNECT:
      this.onconnect();
      break;

    case parser.EVENT:
      this.onevent(packet);
      break;

    case parser.BINARY_EVENT:
      this.onevent(packet);
      break;

    case parser.ACK:
      this.onack(packet);
      break;

    case parser.BINARY_ACK:
      this.onack(packet);
      break;

    case parser.DISCONNECT:
      this.ondisconnect();
      break;

    case parser.ERROR:
      this.emit('error', packet.data);
      break;
  }
}

void SocketIOSocket::onevent(const Packet& packet)
{
  var args = packet.data || [];
  debug('emitting event %j', args);

  if (null != packet.id) {
    debug('attaching ack callback to event');
    args.push(this.ack(packet.id));
  }

  if (_connected) {
    emit.apply(this, args);
  } else {
    _receiveBuffer.push_back(args);
  }
}

void SocketIOSocket::ack(int id)
{
  var self = this;
  var sent = false;
  return function () {
    // prevent double callbacks
    if (sent) return;
    sent = true;
    var args = toArray(arguments);
    debug('sending ack %j', args);

    var type = hasBin(args) ? parser.BINARY_ACK : parser.ACK;
    self.packet({
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
  this.emit("connect");
  this.emitBuffered();
}

void SocketIOSocket::emitBuffered()
{
  size_t i;
  for (i = 0; i < _receiveBuffer.size(); i++) {
    emit.apply(this, _receiveBuffer[i]);
  }
  _receiveBuffer.clear();

  for (i = 0; i < _sendBuffer.length; i++) {
    sendPacket(_sendBuffer[i]);
  }
  _sendBuffer = [];
}

void SocketIOSocket::ondisconnect()
{
  debug('server disconnect (%s)', _nsp);
  destroy();
  this.onclose('io server disconnect');
}

void SocketIOSocket::destroy()
{
  if (_subs) {
    // clean subscriptions to avoid reconnections
    for (var i = 0; i < _subs.length; i++) {
      _subs[i].destroy();
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
    sendPacket({ type: parser.DISCONNECT });
  }

  // remove socket from pool
  destroy();

  if (_connected) {
    // fire events
    this.onclose('io client disconnect');
  }
}

void SocketIOSocket::setCompress(bool compress)
{
    _compress = compress;
}
