/**
 * Internal events (blacklisted).
 * These events can't be emitted by the user.
 *
 * @api private
 */

enum class events = {
  connect: 1,
  connect_error: 1,
  connect_timeout: 1,
  connecting: 1,
  disconnect: 1,
  error: 1,
  reconnect: 1,
  reconnect_attempt: 1,
  reconnect_failed: 1,
  reconnect_error: 1,
  reconnecting: 1,
  ping: 1,
  pong: 1
};


SocketIOSocket::SocketIOSocket(SocketIOManager* io, const std::string& nsp, const Opts& opts)
{
  _io = io;
  _nsp = nsp;
  _ids = 0;
  _acks.clear();
  _receiveBuffer.clear();
  this.sendBuffer = [];
  _connected = false;
  this.disconnected = true;
  if (opts && opts.query) {
    this.query = opts.query;
  }
  if (_io.autoConnect) this.open();
}

void SocketIOSocket::subEvents()
{
  if (this.subs) return;

  var io = _io;
  this.subs = [
    on(io, 'open', bind(this, 'onopen')),
    on(io, 'packet', bind(this, 'onpacket')),
    on(io, 'close', bind(this, 'onclose'))
  ];
}

// connect
void SocketIOSocket::open()
{
  if (_connected) return;

  subEvents();
  _io.open(); // ensure open
  if ('open' === _io.readyState) this.onopen();
  this.emit('connecting');
  return this;
}

void SocketIOSocket::send()
{
  var args = toArray(arguments);
  args.unshift('message');
  this.emit.apply(this, args);
  return this;
}

void SocketIOSocket::emit(const std::string& eventName, const Args& args)
{
  if (events.hasOwnProperty(ev)) {
    emit.apply(this, arguments);
    return this;
  }

  var args = toArray(arguments);
  var parserType = parser.EVENT; // default
  if (hasBin(args)) { parserType = parser.BINARY_EVENT; } // binary
  var packet = { type: parserType, data: args };

  packet.options = {};
  packet.options.compress = !this.flags || false !== this.flags.compress;

  // event ack callback
  if ('function' === typeof args[args.length - 1]) {
    debug('emitting packet with ack id %d', this.ids);
    _acks[this.ids] = args.pop();
    packet.id = this.ids++;
  }

  if (_connected) {
    sendPacket(packet);
  } else {
    this.sendBuffer.push(packet);
  }

  delete this.flags;

  return this;
}

void SocketIOSocket::sendPacket(const Packet& packet)
{
  packet.nsp = this.nsp;
  _io->sendPacket(packet);
}

void SocketIOSocket::onopen()
{
  debug('transport is open - connecting');

  // write connect packet if necessary
  if ('/' !== this.nsp) {
    if (this.query) {
      this.packet({type: parser.CONNECT, query: this.query});
    } else {
      this.packet({type: parser.CONNECT});
    }
  }
}

void SocketIOSocket::onclose(const std::string& reason)
{
  debug('close (%s)', reason);
  _connected = false;
  this.disconnected = true;
  delete this.id;
  this.emit('disconnect', reason);
}

void SocketIOSocket::onpacket(const Packet& packet)
{
  if (packet.nsp !== this.nsp) return;

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
  if ('function' === typeof ack) {
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
  this.disconnected = false;
  this.emit('connect');
  this.emitBuffered();
}

void SocketIOSocket::emitBuffered()
{
  size_t i;
  for (i = 0; i < _receiveBuffer.size(); i++) {
    emit.apply(this, _receiveBuffer[i]);
  }
  _receiveBuffer.clear();

  for (i = 0; i < this.sendBuffer.length; i++) {
    this.packet(this.sendBuffer[i]);
  }
  this.sendBuffer = [];
}

void SocketIOSocket::ondisconnect()
{
  debug('server disconnect (%s)', this.nsp);
  this.destroy();
  this.onclose('io server disconnect');
}

void SocketIOSocket::destroy()
{
  if (this.subs) {
    // clean subscriptions to avoid reconnections
    for (var i = 0; i < this.subs.length; i++) {
      this.subs[i].destroy();
    }
    this.subs = null;
  }

  _io.destroy(this);
}

// disconnect
void SocketIOSocket::close()
{
  if (_connected) {
    debug('performing disconnect (%s)', this.nsp);
    this.packet({ type: parser.DISCONNECT });
  }

  // remove socket from pool
  this.destroy();

  if (_connected) {
    // fire events
    this.onclose('io client disconnect');
  }
  return this;
}

void SocketIOSocket::compress(bool isCompress)
{
  this.flags = this.flags || {};
  this.flags.compress = isCompress;
}
