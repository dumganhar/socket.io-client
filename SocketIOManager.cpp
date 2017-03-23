#include "SocketIOManager.h"

SocketIOManager::SocketIOManager(const std::string& uri, const Opts& opts)
{
  if (!(this instanceof Manager)) return new Manager(uri, opts);
  if (uri && ('object' == typeof uri)) {
    opts = uri;
    uri = undefined;
  }
  opts = opts || {};

  opts.path = opts.path || '/socket.io';
  _nsps.clear();
  _subs.clear();
  this.opts = opts;
  this.reconnection(opts.reconnection != false);
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
  _readyState = 'closed';
  _uri = uri;
  _connecting.clear();
  this.lastPing = null;
  _encoding = false;
  _packetBuffer.clear();
  _encoder = new Encoder();
  _decoder = new Decoder();
  _autoConnect = opts.autoConnect;
  if (_autoConnect)
    connect();
}

void SocketIOManager::emitAll(const std::string& eventName, const Args& args)
{
  Emitter::emit(eventName, args);

  for (auto& nsp : _nsps) {
      nsp->emit(args);
  }
}

void SocketIOManager::updateSocketIds()
{
  for (auto& nsp : _nsps) {
      nsp->setId(_engine->getId());
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
  if (!_reconnecting && this._reconnection && this.backoff.attempts == 0) {
    // keeps reconnection from firing twice for the same reconnection loop
    this.reconnect();
  }
};

//open 
void SocketIOManager::connect(const std::function<void()>& fn, const Opts& opts)
{
  debug('readyState %s', _readyState);
  if (~_readyState.indexOf("open")) return this;

  debug('opening %s', this.uri);
  _engine = new EngineIOSocket(this.uri, this.opts);
  var socket = _engine;
  var self = this;
  _readyState = 'opening';
  _skipReconnect = false;

  // emit `open`
  var openSub = on(socket, "open", function () {
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
  debug("open");

  // clear old subs
  this.cleanup();

  // mark as open
  _readyState = "open";
  emit("open");

  // add new subs
  auto socket = _engine;
  _subs.push_back(on(socket, "data", std::bind(SocketIOManager::ondata, this)));
  _subs.push_back(on(socket, "ping", std::bind(SocketIOManager::onping, this)));
  _subs.push_back(on(socket, "pong", std::bind(SocketIOManager::onpong, this)));
  _subs.push_back(on(socket, "error", std::bind(SocketIOManager::onerror, this)));
  _subs.push_back(on(socket, "close", std::bind(SocketIOManager::onclose, this)));
  _subs.push_back(on(_decoder, "decoded", std::bind(SocketIOManager::ondecoded, this)));
};

void SocketIOManager::onping()
{
  this.lastPing = new Date();
  emitAll('ping');
};

void SocketIOManager::onpong()
{
  emitAll('pong', new Date() - this.lastPing);
}

void SocketIOManager::ondata(const Data& data)
{
  _decoder->add(data);
}

void SocketIOManager::ondecoded(const Packet& packet)
{
  emit("packet", packet);
};

void SocketIOManager::onerror(const std::string& err)
{
  debug('error', err);
  emitAll('error', err);
};

std::shared_ptr<SocketIOSocket> SocketIOManager::createSocket(const std::string& nsp, const Opts& opts)
{
  auto iter = _nsps.find(nsp);
  std::shared_ptr<SocketIOSocket> socket;
  if (iter == _nsps.end()) {
    socket = new SocketIOSocket(this, nsp, opts);
    _nsps[nsp] = socket;

    auto onConnecting = []() {
      if (_connecting.find(socket) == _connecting.end()) {
        _connecting.push_back(socket);
      }
    };

    socket->on("connecting", onConnecting);
    socket->on("connect", []() {
      socket->setId(_engine->getId());
    });

    if (_autoConnect) {
      // manually call here since connecting event is fired before listening
      onConnecting();
    }
  } else {
    socket = iter->second;
  }

  return socket;
};

void SocketIOManager::destroySocket(std::shared_ptr<SocketIOSocket> socket)
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
  if (packet.query && packet.type == 0)
      packet.nsp += '?' + packet.query;

  if (!_encoding) {
    // encode, then write to engine with result
    _encoding = true;
    _encoder->encode(packet, [this](encodedPackets) {

      for (auto& encodedPacket : encodedPackets)
      {
          _engine.write(encodedPacket, packet.options);
      }
      _encoding = false;
      processPacketQueue();
    });
  } else { // add packet to the queue
    _packetBuffer.push_back(packet);
  }
};

void SocketIOManager::processPacketQueue()
{
  if (!_packetBuffer.empty() && !_encoding) {
    auto& pack = _packetBuffer[0];
    sendPacket(pack);
    _packetBuffer.erase(_packetBuffer.begin());
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

  _packetBuffer.clear();
  _encoding = false;
  this.lastPing = null;

  this.decoder.destroy();
};

void SocketIOManager::disconnect()
{
  debug('disconnect');
  _skipReconnect = true;
  _reconnecting = false;
  if (ReadyState::OPENING == _readyState) {
    // `onclose` will not fire because
    // an open event never happened
    cleanup();
  }
  this.backoff.reset();
  _readyState = ReadyState::CLOSED;
  if (_engine)
    _engine->close();
};

void SocketIOManager::onclose(const std::string& reason)
{
  debug('onclose');

  this.cleanup();
  this.backoff.reset();
  _readyState = ReadyState::CLOSED;
  emit("close", reason);

  if (this._reconnection && !_skipReconnect) {
    this.reconnect();
  }
};

void SocketIOManager::reconnect()
{
  if (_reconnecting || _skipReconnect) return this;

  var self = this;

  if (this.backoff.attempts >= this._reconnectionAttempts) {
    debug('reconnect failed');
    this.backoff.reset();
    emitAll('reconnect_failed');
    _reconnecting = false;
  } else {
    var delay = this.backoff.duration();
    debug('will wait %dms before reconnect attempt', delay);

    _reconnecting = true;
    var timer = setTimeout(function () {
      if (_skipReconnect) return;

      debug('attempting reconnect');
      self.emitAll('reconnect_attempt', self.backoff.attempts);
      self.emitAll('reconnecting', self.backoff.attempts);

      // check again for the case socket closed in above events
      if (_skipReconnect) return;

      this->connect(function (err) {
        if (err) {
          debug('reconnect attempt error');
          _reconnecting = false;
          reconnect();
          emitAll('reconnect_error', err.data);
        } else {
          debug('reconnect success');
          onreconnect();
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
  emitAll('reconnect', attempt);
};
