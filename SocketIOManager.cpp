#include "SocketIOManager.h"

SocketIOManager::SocketIOManager(const std::string& uri, const Opts& opts)
{
  if (!(this instanceof Manager)) return new Manager(uri, opts);
  if (uri && ('object' === typeof uri)) {
    opts = uri;
    uri = undefined;
  }
  opts = opts || {};

  opts.path = opts.path || '/socket.io';
  _nsps.clear();
  _subs.clear();
  this.opts = opts;
  this.reconnection(opts.reconnection !== false);
  this.reconnectionAttempts(opts.reconnectionAttempts || Infinity);
  this.reconnectionDelay(opts.reconnectionDelay || 1000);
  this.reconnectionDelayMax(opts.reconnectionDelayMax || 5000);
  this.randomizationFactor(opts.randomizationFactor || 0.5);
  this.backoff = new Backoff({
    min: this.reconnectionDelay(),
    max: this.reconnectionDelayMax(),
    jitter: this.randomizationFactor()
  });
  this.timeout(null == opts.timeout ? 20000 : opts.timeout);
  this.readyState = 'closed';
  _uri = uri;
  _connecting.clear();
  this.lastPing = null;
  this.encoding = false;
  this.packetBuffer = [];
  this.encoder = new parser.Encoder();
  this.decoder = new parser.Decoder();
  this.autoConnect = opts.autoConnect !== false;
  if (this.autoConnect) this.connect();
}

void SocketIOManager::emitAll()
{
  this.emit.apply(this, arguments);
  for (var nsp in _nsps) {
    if (has.call(_nsps, nsp)) {
      _nsps[nsp].emit.apply(_nsps[nsp], arguments);
    }
  }
};

void SocketIOManager::updateSocketIds()
{
  for (var nsp in _nsps) {
    if (has.call(_nsps, nsp)) {
      _nsps[nsp].id = _engine.id;
    }
  }
};


void SocketIOManager::reconnection(v)
{
  if (!arguments.length) return this._reconnection;
  this._reconnection = !!v;
};

void SocketIOManager::reconnectionAttempts(int v)
{
  if (!arguments.length) return this._reconnectionAttempts;
  this._reconnectionAttempts = v;
};

void SocketIOManager::reconnectionDelay(float v)
{
  if (!arguments.length) return this._reconnectionDelay;
  this._reconnectionDelay = v;
  this.backoff && this.backoff.setMin(v);
  return this;
}

Manager.prototype.randomizationFactor = function (v) {
  if (!arguments.length) return this._randomizationFactor;
  this._randomizationFactor = v;
  this.backoff && this.backoff.setJitter(v);
  return this;
};

void SocketIOManager::reconnectionDelayMax(float v)
{
  if (!arguments.length) return this._reconnectionDelayMax;
  this._reconnectionDelayMax = v;
  this.backoff && this.backoff.setMax(v);
  return this;
};

void SocketIOManager::enableTimeout(bool v)
{
  if (!arguments.length) return _timeout;
  _timeout = v;
}

bool SocketIOManager::isTimeoutEnabled() const
{
  return _timeout;
}

void SocketIOManager::maybeReconnectOnOpen()
{
  // Only try to reconnect if it's the first time we're connecting
  if (!_reconnecting && this._reconnection && this.backoff.attempts === 0) {
    // keeps reconnection from firing twice for the same reconnection loop
    this.reconnect();
  }
};

//open 
void SocketIOManager::connect(const std::function<void()>& fn, const Opts& opts)
{
  debug('readyState %s', this.readyState);
  if (~this.readyState.indexOf('open')) return this;

  debug('opening %s', this.uri);
  _engine = new EngineIOSocket(this.uri, this.opts);
  var socket = _engine;
  var self = this;
  this.readyState = 'opening';
  this.skipReconnect = false;

  // emit `open`
  var openSub = on(socket, 'open', function () {
    self.onopen();
    fn && fn();
  });

  // emit `connect_error`
  var errorSub = on(socket, 'error', function (data) {
    debug('connect_error');
    self.cleanup();
    self.readyState = 'closed';
    self.emitAll('connect_error', data);
    if (fn) {
      var err = new Error('Connection error');
      err.data = data;
      fn(err);
    } else {
      // Only do this if there is no fn to handle the error
      self.maybeReconnectOnOpen();
    }
  });

  // emit `connect_timeout`
  if (false != _timeout) {
    var timeout = _timeout;
    debug('connect attempt will timeout after %d', timeout);

    // set timer
    var timer = setTimeout(function () {
      debug('connect attempt timed out after %d', timeout);
      openSub.destroy();
      socket.close();
      socket.emit('error', 'timeout');
      self.emitAll('connect_timeout', timeout);
    }, timeout);

    OnObj onObj;
    onObj.destroy = [](){
      clearTimeout(timer);
    };
    _subs.push_back(std::move(onObj));
  }

  _subs.push_back(openSub);
  _subs.push_back(errorSub);

  return this;
};

void SocketIOManager::onopen()
{
  debug('open');

  // clear old subs
  this.cleanup();

  // mark as open
  this.readyState = 'open';
  this.emit('open');

  // add new subs
  var socket = _engine;
  _subs.push(on(socket, 'data', bind(this, 'ondata')));
  _subs.push(on(socket, 'ping', bind(this, 'onping')));
  _subs.push(on(socket, 'pong', bind(this, 'onpong')));
  _subs.push(on(socket, 'error', bind(this, 'onerror')));
  _subs.push(on(socket, 'close', bind(this, 'onclose')));
  _subs.push(on(this.decoder, 'decoded', bind(this, 'ondecoded')));
};

void SocketIOManager::onping()
{
  this.lastPing = new Date();
  this.emitAll('ping');
};

void SocketIOManager::onpong()
{
  this.emitAll('pong', new Date() - this.lastPing);
}

void SocketIOManager::ondata(const Data& data)
{
  this.decoder.add(data);
}

void SocketIOManager::ondecoded(const Packet& packet)
{
  this.emit('packet', packet);
};

void SocketIOManager::onerror(const std::string& err)
{
  debug('error', err);
  this.emitAll('error', err);
};

SocketIOSocket* SocketIOManager::createSocket(const std::string& nsp, const Opts& opts)
{
  auto iter = _nsps.find(nsp);
  SocketIOSocket* socket = nullptr;
  if (iter == _nsps.end()) {
    socket = new SocketIOSocket(this, nsp, opts);
    _nsps[nsp] = socket;
    var self = this;
    socket.on('connecting', onConnecting);
    socket.on('connect', function () {
      socket.id = _engine.id;
    });

    if (_autoConnect) {
      // manually call here since connecting event is fired before listening
      onConnecting();
    }
  } else {
    socket = iter->second;
  }

  function onConnecting () {
    if (!~indexOf(_connecting, socket)) {
      _connecting.push_back(socket);
    }
  }

  return socket;
};

void SocketIOManager::destroySocket(SocketIOSocket* socket)
{
  auto iter = _connecting.find(socket);
  if (iter != _connecting.end())
  {
    _connecting.erase(iter);
  }

  if (!_connecting.empty())
    return;

  disconnect();
};

void SocketIOManager::sendPacket(const Packet& packet)
{
  debug('writing packet %j', packet);
  var self = this;
  if (packet.query && packet.type === 0) packet.nsp += '?' + packet.query;

  if (!self.encoding) {
    // encode, then write to engine with result
    self.encoding = true;
    this.encoder.encode(packet, function (encodedPackets) {
      for (var i = 0; i < encodedPackets.length; i++) {
        _engine.write(encodedPackets[i], packet.options);
      }
      self.encoding = false;
      self.processPacketQueue();
    });
  } else { // add packet to the queue
    self.packetBuffer.push(packet);
  }
};

void SocketIOManager::processPacketQueue()
{
  if (this.packetBuffer.length > 0 && !this.encoding) {
    var pack = this.packetBuffer.shift();
    this.packet(pack);
  }
};

void SocketIOManager::cleanup()
{
  debug('cleanup');

  var subsLength = _subs.length;
  for (var i = 0; i < subsLength; i++) {
    var sub = _subs.shift();
    sub.destroy();
  }

  this.packetBuffer = [];
  this.encoding = false;
  this.lastPing = null;

  this.decoder.destroy();
};

void SocketIOManager::disconnect()
{
  debug('disconnect');
  this.skipReconnect = true;
  _reconnecting = false;
  if ('opening' === this.readyState) {
    // `onclose` will not fire because
    // an open event never happened
    this.cleanup();
  }
  this.backoff.reset();
  this.readyState = 'closed';
  if (_engine) _engine.close();
};

void SocketIOManager::onclose(const std::string& reason)
{
  debug('onclose');

  this.cleanup();
  this.backoff.reset();
  this.readyState = 'closed';
  this.emit('close', reason);

  if (this._reconnection && !this.skipReconnect) {
    this.reconnect();
  }
};

void SocketIOManager::reconnect()
{
  if (_reconnecting || this.skipReconnect) return this;

  var self = this;

  if (this.backoff.attempts >= this._reconnectionAttempts) {
    debug('reconnect failed');
    this.backoff.reset();
    this.emitAll('reconnect_failed');
    _reconnecting = false;
  } else {
    var delay = this.backoff.duration();
    debug('will wait %dms before reconnect attempt', delay);

    _reconnecting = true;
    var timer = setTimeout(function () {
      if (self.skipReconnect) return;

      debug('attempting reconnect');
      self.emitAll('reconnect_attempt', self.backoff.attempts);
      self.emitAll('reconnecting', self.backoff.attempts);

      // check again for the case socket closed in above events
      if (self.skipReconnect) return;

      this->connect(function (err) {
        if (err) {
          debug('reconnect attempt error');
          _reconnecting = false;
          self.reconnect();
          self.emitAll('reconnect_error', err.data);
        } else {
          debug('reconnect success');
          self.onreconnect();
        }
      });
    }, delay);

    OnObj onObj;
    onObj.destroy = [](){
      clearTimeout(timer);
    };
    _subs.push_back(std::move(onObj));
  }
};

void SocketIOManager::onreconnect()
{
  var attempt = this.backoff.attempts;
  _reconnecting = false;
  this.backoff.reset();
  this.updateSocketIds();
  this.emitAll('reconnect', attempt);
};
