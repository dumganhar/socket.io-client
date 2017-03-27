#include "SocketIOManager.h"
#include "SocketIOSocket.h"
#include "SocketIOParser.h"
#include "EngineIOSocket.h"
#include "Backoff.h"
#include "IOUtils.h"

using namespace socketio::parser;

SocketIOManager::SocketIOManager(const std::string& uri, const Opts& opts)
{
  _opts = opts;
  if (_opts.path.empty())
    _opts.path = "/socket.io";

  _nsps.clear();
  _subs.clear();
  
  setAutoReconnect(opts.reconnection);
  setReconnectionAttempts(opts.reconnectionAttempts);
  setReconnectionDelay(opts.reconnectionDelay);
  setReconnectionDelayMax(opts.reconnectionDelayMax);
  setRandomizationFactor(opts.randomizationFactor);
  _backoff.reset(new Backoff(
    /*min:*/ getReconnectionDelay(),
    /*max:*/ getReconnectionDelayMax(),
    /*jitter:*/ getRandomizationFactor(),
                         2
  ));
  setTimeoutDelay(opts.timeout);
  _readyState = ReadyState::CLOSED;
  _uri = uri;
  _connecting.clear();
//cjh  this.lastPing = null;
  _encoding = false;
  _packetBuffer.clear();
  _encoder.reset(new Encoder());
  _decoder.reset(new Decoder());
  _autoConnect = opts.autoConnect;
  if (_autoConnect)
    connect(nullptr, opts);
}

void SocketIOManager::emitAll(const std::string& eventName, const Value& args)
{
  Emitter::emit(eventName, args);

  for (const auto& e : _nsps) {
      e.second->emit(args);
  }
}

void SocketIOManager::updateSocketIds()
{
  for (const auto& e : _nsps) {
      e.second->setId(_engine->getId());
  }
}

void SocketIOManager::setAutoReconnect(bool autoReconnect)
{
    _reconnection = autoReconnect;
}

bool SocketIOManager::isAutoReconnect() const
{
  return _reconnection;
}

void SocketIOManager::setReconnectionAttempts(int v)
{
  _reconnectionAttempts = v;
}

int SocketIOManager::getReconnectionAttempts() const
{
  return _reconnectionAttempts;
}

void SocketIOManager::setReconnectionDelay(long v)
{
  _reconnectionDelay = v;
  _backoff->setMin(v);
}

long SocketIOManager::getReconnectionDelay() const
{
  return _reconnectionDelay;
}

void SocketIOManager::setRandomizationFactor(float v)
{
  _randomizationFactor = v;
  _backoff->setJitter(v);
}

float SocketIOManager::getRandomizationFactor() const
{
  return _randomizationFactor;
}

void SocketIOManager::setReconnectionDelayMax(long v)
{
  _reconnectionDelayMax = v;
  _backoff->setMax(v);
}

long SocketIOManager::getReconnectionDelayMax() const
{
  return _reconnectionDelayMax;
}

void SocketIOManager::setTimeoutDelay(long v)
{
    _timeout = v;
}

long SocketIOManager::getTimeoutDelay() const
{
  return _timeout;
}

void SocketIOManager::maybeReconnectOnOpen()
{
  // Only try to reconnect if it"s the first time we"re connecting
  if (!_reconnecting && _reconnection && _backoff->getAttempts() == 0) {
    // keeps reconnection from firing twice for the same reconnection loop
    reconnect();
  }
};

//open 
void SocketIOManager::connect(const std::function<void()>& fn, const Opts& opts)
{
  debug("readyState %d", (int)_readyState);
  if (_readyState == ReadyState::OPENED)
    return;

  debug("opening %s", _uri.c_str());
  _engine = std::make_shared<EngineIOSocket>(_uri, _opts);
  auto socket = _engine;
  _readyState = ReadyState::OPENING;
  _skipReconnect = false;

  // emit `open`
  OnObj openSub = gon(socket, "open", [this, fn](const Value& v) {
      onopen(Value::NONE);
    if (fn)
        fn();
  }, ID());

  // emit `connect_error`
  OnObj errorSub = gon(socket, "error", [this, fn](const Value& data) {
    debug("connect_error");
    cleanup();
    _readyState = ReadyState::CLOSED;
    emitAll("connect_error", data);
    if (fn) {
//cjh      var err = new Error("Connection error");
//      err.data = data;
//      fn(err);
    } else {
      // Only do this if there is no fn to handle the error
      maybeReconnectOnOpen();
    }
  }, ID());

  // emit `connect_timeout`
  if (false != _timeout) {
    float timeout = _timeout;
    debug("connect attempt will timeout after %f", timeout);

    // set timer
    TimerHandle timer = setTimeout([=]() {
      debug("connect attempt timed out after %f", timeout);
      openSub.destroy();
      socket->close();
      socket->emit("error", "timeout");
//cjh      emitAll("connect_timeout", timeout);
    }, timeout);

    OnObj onObj;
    onObj.destroy = [=](){
      clearTimeout(timer);
    };
    _subs.push_back(std::move(onObj));
  }

  _subs.push_back(openSub);
  _subs.push_back(errorSub);
}

void SocketIOManager::onopen(const Value& unused)
{
  debug("open");

  // clear old subs
  cleanup();

  // mark as open
  _readyState = ReadyState::OPENED;
  emit("open");

  // add new subs
  auto socket = _engine;
  _subs.push_back(gon(socket, "data", std::bind(&SocketIOManager::ondata, this, std::placeholders::_1)));
  _subs.push_back(gon(socket, "ping", std::bind(&SocketIOManager::onping, this, std::placeholders::_1)));
  _subs.push_back(gon(socket, "pong", std::bind(&SocketIOManager::onpong, this, std::placeholders::_1)));
  _subs.push_back(gon(socket, "error", std::bind(&SocketIOManager::onerror, this, std::placeholders::_1)));
  _subs.push_back(gon(socket, "close", std::bind(&SocketIOManager::onclose, this, std::placeholders::_1)));
  _subs.push_back(gon(_decoder, "decoded", std::bind(&SocketIOManager::ondecoded, this, std::placeholders::_1)));
}

void SocketIOManager::onping(const Value& unused)
{
//cjh  this.lastPing = new Date();
  emitAll("ping");
};

void SocketIOManager::onpong(const Value& unused)
{
//cjh  emitAll("pong", new Date() - this.lastPing);
}

void SocketIOManager::ondata(const Value& data)
{
  _decoder->add(data);
}

void SocketIOManager::ondecoded(const Value& packet)
{
  emit("packet", packet);
};

void SocketIOManager::onerror(const Value& err)
{
  debug("error: %s", err.asString().c_str());
  emitAll("error", err);
};

std::shared_ptr<SocketIOSocket> SocketIOManager::createSocket(const std::string& nsp, const Opts& opts)
{
  auto iter = _nsps.find(nsp);
  std::shared_ptr<SocketIOSocket> socket;
  if (iter == _nsps.end()) {
      socket = std::make_shared<SocketIOSocket>(shared_from_this(), nsp, opts);
    _nsps[nsp] = socket;

    auto onConnecting = [=](const Value& used) {
        if (std::find(_connecting.begin(), _connecting.end(), socket) == _connecting.end()) {
        _connecting.push_back(socket);
      }
    };

    socket->on("connecting", onConnecting);
    socket->on("connect", [=](const Value& used) {
      socket->setId(_engine->getId());
    });

    if (_autoConnect) {
      // manually call here since connecting event is fired before listening
        onConnecting(Value::NONE);
    }
  } else {
    socket = iter->second;
  }

  return socket;
};

void SocketIOManager::destroySocket(std::shared_ptr<SocketIOSocket> socket)
{
    auto iter = std::find(_connecting.begin(), _connecting.end(), socket);
  if (iter != _connecting.end())
  {
    _connecting.erase(iter);
  }

  if (!_connecting.empty())
    return;

  disconnect();
};

void SocketIOManager::sendPacket(SocketIOPacket& packet)
{
  debug("writing packet %s", packet.toString().c_str());
    if (!packet.query.empty() && packet.type == SocketIOPacket::Type::CONNECT)
    {
        packet.nsp += ("?" + packet.query);
    }

  if (!_encoding) {
    // encode, then write to engine with result
    _encoding = true;
    ValueArray encodedPackets = _encoder->encode(packet);

//    for (const auto& encodedPacket : encodedPackets)
//    {
//cjh        _engine->send(encodedPacket, packet.options);
//    }
    _encoding = false;
    processPacketQueue();

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
  debug("cleanup");

    for (const auto& sub : _subs)
    {
        sub.destroy();
    }

    _subs.clear();

  _packetBuffer.clear();
  _encoding = false;
//cjh  this.lastPing = null;

  _decoder->destroy();
};

void SocketIOManager::disconnect()
{
  debug("disconnect");
  _skipReconnect = true;
  _reconnecting = false;
  if (ReadyState::OPENING == _readyState) {
    // `onclose` will not fire because
    // an open event never happened
    cleanup();
  }
  _backoff->reset();
  _readyState = ReadyState::CLOSED;
  if (_engine)
    _engine->close();
};

void SocketIOManager::onclose(const Value& reason)
{
  debug("onclose");

  cleanup();
  _backoff->reset();
  _readyState = ReadyState::CLOSED;
  emit("close", reason);

  if (_reconnection && !_skipReconnect) {
    reconnect();
  }
};

void SocketIOManager::reconnect()
{
  if (_reconnecting || _skipReconnect)
    return;

  if (_backoff->getAttempts() >= _reconnectionAttempts) {
    debug("reconnect failed");
    _backoff->reset();
    emitAll("reconnect_failed");
    _reconnecting = false;
  } else {
    long delay = _backoff->getDuration();
    debug("will wait %ldms before reconnect attempt", delay);

    _reconnecting = true;
    TimerHandle timer = setTimeout([=] () {
      if (_skipReconnect) return;

      debug("attempting reconnect");
//cjh      emitAll("reconnect_attempt", _backoff->getAttempts());
//      emitAll("reconnecting", _backoff->getAttempts());

      // check again for the case socket closed in above events
      if (_skipReconnect) return;

//cjh      this->connect([](err) {
//        if (err) {
//          debug("reconnect attempt error");
//          _reconnecting = false;
//          reconnect();
//          emitAll("reconnect_error", err.data);
//        } else {
//          debug("reconnect success");
//          onreconnect();
//        }
//      });
    }, delay);

    OnObj onObj;
    onObj.destroy = [=](){
      clearTimeout(timer);
    };
    _subs.push_back(std::move(onObj));
  }
}

void SocketIOManager::onreconnect()
{
  int attempt = _backoff->getAttempts();
  _reconnecting = false;
  _backoff->reset();
  updateSocketIds();
  emitAll("reconnect", Value(attempt));
}
