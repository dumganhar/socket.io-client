#include "EngineIOSocket.h"
#include "EngineIOParser.h"
#include "EngineIOTransport.h"
#include "IOUtils.h"

static bool __priorWebsocketSuccess = false;

EngineIOSocket::EngineIOSocket(const std::string& uri, const Opts& opts)
{
//  if (uri) {
//    uri = parseuri(uri);
//    opts.hostname = uri.host;
//    opts.secure = uri.protocol == "https' || uri.protocol == 'wss";
//    opts.port = uri.port;
//    if (uri.query) opts.query = uri.query;
//  } else if (opts.host) {
//    opts.hostname = parseuri(opts.host).host;
//  }
//
//  this.secure = null != opts.secure ? opts.secure
//    : (global.location && "https:" == location.protocol);
//
//  if (opts.hostname && !opts.port) {
//    // if no port is specified manually, use the protocol default
//    opts.port = this.secure ? "443' : '80";
//  }
//
//  this.agent = opts.agent || false;
//  this.hostname = opts.hostname ||
//    (global.location ? location.hostname : "localhost");
//  this.port = opts.port || (global.location && location.port
//      ? location.port
//      : (this.secure ? 443 : 80));
//  this.query = opts.query || {};
//  if ("string" == typeof this.query) this.query = parseqs.decode(this.query);
//  _upgrade = false != opts.upgrade;
//  this.path = (opts.path || "/engine.io').replace(/\/$/, '') + '/";
//  this.forceJSONP = !!opts.forceJSONP;
//  this.jsonp = false != opts.jsonp;
//  this.forceBase64 = !!opts.forceBase64;
//  this.enablesXDR = !!opts.enablesXDR;
//  this.timestampParam = opts.timestampParam || "t";
//  this.timestampRequests = opts.timestampRequests;
//  _transports = opts.transports || ["polling', 'websocket"];
//  _readyState = ReadyState::NONE;
//  _writeBuffer.clear();
//  _prevBufferLen = 0;
//  this.policyPort = opts.policyPort || 843;
//  _rememberUpgrade = opts.rememberUpgrade || false;
//  this.binaryType = null;
//  this.onlyBinaryUpgrades = opts.onlyBinaryUpgrades;
//  this.perMessageDeflate = false != opts.perMessageDeflate ? (opts.perMessageDeflate || {}) : false;
//
//  if (true == this.perMessageDeflate) this.perMessageDeflate = {};
//  if (this.perMessageDeflate && null == this.perMessageDeflate.threshold) {
//    this.perMessageDeflate.threshold = 1024;
//  }
//
//  // SSL options for Node.js client
//  this.pfx = opts.pfx || null;
//  this.key = opts.key || null;
//  this.passphrase = opts.passphrase || null;
//  this.cert = opts.cert || null;
//  this.ca = opts.ca || null;
//  this.ciphers = opts.ciphers || null;
//  this.rejectUnauthorized = opts.rejectUnauthorized == undefined ? null : opts.rejectUnauthorized;
//  this.forceNode = !!opts.forceNode;
//
//  // other options for Node.js client
//  var freeGlobal = typeof global == "object" && global;
//  if (freeGlobal.global == freeGlobal) {
//    if (opts.extraHeaders && Object.keys(opts.extraHeaders).length > 0) {
//      this.extraHeaders = opts.extraHeaders;
//    }
//
//    if (opts.localAddress) {
//      this.localAddress = opts.localAddress;
//    }
//  }
//
  // set on handshake
  _id = "";
  _upgrades.clear();
  _pingInterval = 0;
  _pingTimeout = 0;

//  // set on heartbeat
  _pingIntervalTimer = INVALID_TIMER_HANDLE;
  _pingTimeoutTimer = INVALID_TIMER_HANDLE;

  open();
}

int EngineIOSocket::getProtocolVersion() const
{
    return engineio::parser::getProtocolVersion();
}

/**
 * Creates transport of the given type.
 *
 * @param {String} transport name
 * @return {Transport}
 * @api private
 */

std::shared_ptr<EngineIOTransport> EngineIOSocket::createTransport(const std::string& name)
{
    debug("creating transport %s", name.c_str());
    ValueObject query = _query;

    // append engine.io protocol identifier
    query["EIO"] = engineio::parser::getProtocolVersion();

    // transport name
    query["transport"] = name;

    // session id if we already have one
    if (!_id.empty())
        query["sid"] = _id;

    ValueObject opts;
    auto transport = EngineIOTransport::create(name, opts);
//  auto transport = new transports[name]({
//    agent: this.agent,
//    hostname: this.hostname,
//    port: this.port,
//    secure: this.secure,
//    path: this.path,
//    query: query,
//    forceJSONP: this.forceJSONP,
//    jsonp: this.jsonp,
//    forceBase64: this.forceBase64,
//    enablesXDR: this.enablesXDR,
//    timestampRequests: this.timestampRequests,
//    timestampParam: this.timestampParam,
//    policyPort: this.policyPort,
//    socket: this,
//    pfx: this.pfx,
//    key: this.key,
//    passphrase: this.passphrase,
//    cert: this.cert,
//    ca: this.ca,
//    ciphers: this.ciphers,
//    rejectUnauthorized: this.rejectUnauthorized,
//    perMessageDeflate: this.perMessageDeflate,
//    extraHeaders: this.extraHeaders,
//    forceNode: this.forceNode,
//    localAddress: this.localAddress
//  });
//
  return transport;
}

//cjh function clone (obj) {
//  var o = {};
//  for (var i in obj) {
//    if (obj.hasOwnProperty(i)) {
//      o[i] = obj[i];
//    }
//  }
//  return o;
//}

void EngineIOSocket::open()
{
  std::string transportName;
    if (_rememberUpgrade && __priorWebsocketSuccess && std::find(_transports.begin(), _transports.end(), "websocket") != _transports.end()) {
    transportName = "websocket";
  } else if (_transports.empty()) {
    // Emit error on next tick so it can be listened to
    setTimeout([this]() {
      this->emit("error", "No transports available");
    }, 0);
    return;
  } else {
    transportName = _transports[0];
  }
  _readyState = ReadyState::OPENING;

  // Retry with the next transport if the transport is disabled (jsonp: false)
  std::shared_ptr<EngineIOTransport> transport = createTransport(transportName);

  if (transport == nullptr) {
      _transports.erase(_transports.begin());
      open();
      return;
  }

  transport->open();
  setTransport(transport);
}

void EngineIOSocket::setTransport(std::shared_ptr<EngineIOTransport> transport)
{
  debug("setting transport %s", transport->getName().c_str());

  if (_transport) {
    debug("clearing existing transport %s", _transport->getName().c_str());
    _transport->offAll();
  }

  // set up transport
  _transport = transport;

  // set up transport listeners
  transport->on("drain", [this](const Value&) {
    this->onDrain();
  });

  transport->on("packet", [this](const Value& v) {
      const EngineIOPacket& packet = v.asEngineIOPacket();
      this->onPacket(packet);
  });

  transport->on("error", [this](const Value& e) {
//cjh    this->onError(e);
  });

  transport->on("close", [this](const Value&) {
      this->onClose("transport close", "");
  });
}

void EngineIOSocket::probe(const std::string& name)
{
  debug("probing transport %s", name.c_str());
    std::shared_ptr<EngineIOTransport> transport = createTransport(name);//, { probe: 1 });
    std::shared_ptr<bool> failed = std::make_shared<bool>(false);

  __priorWebsocketSuccess = false;

  // Remove all listeners on the transport and on self
  auto cleanup = [=]() {
    transport->off("open", _idOnTransportOpen);
    transport->off("error", _idOnerror);
    transport->off("close", _idOnTransportClose);
    off("close", _idOnclose);
    off("upgrading", _idOnupgrade);
  };

  auto onTransportOpen = [=](const Value&) {
    if (_onlyBinaryUpgrades) {
        bool upgradeLosesBinary = !_supportsBinary;//cjh && _transport.supportsBinary;
        *failed = (*failed || upgradeLosesBinary);
    }
    if (*failed) return;

    debug("probe transport %s opened", name.c_str());
      EngineIOPacket p;
      p.type = "ping";
      p.data = "probe";
      std::vector<EngineIOPacket> packets;
      packets.push_back(std::move(p));
      transport->send(packets);

    transport->once("packet", [=](const Value& msg) {
      if (*failed) return;

        const EngineIOPacket& packet = msg.asEngineIOPacket();

      if ("pong" == packet.type && "probe" == packet.data.asString()) {
        debug("probe transport %s pong", name.c_str());
        _upgrading = true;
//cjh        emit("upgrading", transport);
        if (!transport) return;
        __priorWebsocketSuccess = "websocket" == transport->getName();

        debug("pausing current transport %s", transport->getName().c_str());
        transport->pause([=]() {
          if (failed) return;
          if (ReadyState::CLOSED == _readyState) return;
          debug("changing transport and sending upgrade packet");

          cleanup();

          setTransport(transport);
            EngineIOPacket p2;
            p2.type = "upgrade";

            std::vector<EngineIOPacket> ps;
            ps.push_back(p2);

            transport->send(ps);
//cjh          emit("upgrade", transport);
//          transport = nullptr;
          _upgrading = false;
          flush();
        });
      } else {
        debug("probe transport %s failed", name.c_str());
//        var err = new Error("probe error");
//        err.transport = transport->getName();
//        emit("upgradeError", err);
      }
    });
  };

  auto freezeTransport = [=]() {
    if (*failed) return;

    // Any callback called by transport should be ignored since now
    *failed = true;

    cleanup();

    transport->close();
//cjh    transport = nullptr;
  };

  // Handle any error that happens while probing
  auto onerror = [=](const Value& err) {
//    var error = new Error("probe error: " + err);
//    error.transport = transport->getName();

    freezeTransport();

//cjh    debug("probe transport %s failed because of error: %s", name.c_str(), err);

//cjh    emit("upgradeError", error);
  };

  auto onTransportClose = [=](const Value&) {
    onerror("transport closed");
  };

  // When the socket is closed while we're probing
  auto onclose = [=](const Value&) {
    onerror("socket closed");
  };

  // When the socket is upgraded while we're probing
  auto onupgrade = [=](const Value& to) {
      const std::string& name = to.asString();
    if (transport && name != transport->getName()) {
      debug("%s works - aborting %s", name.c_str(), transport->getName().c_str());
      freezeTransport();
    }
  };

  transport->once("open", onTransportOpen, ID(&_idOnTransportOpen));
  transport->once("error", onerror, ID(&_idOnerror));
  transport->once("close", onTransportClose, ID(&_idOnTransportClose));

  once("close", onclose, ID(&_idOnclose));
  once("upgrading", onupgrade, ID(&_idOnupgrade));

  transport->open();
}

void EngineIOSocket::onOpen()
{
  debug("socket open");
  _readyState = ReadyState::OPENED;
  __priorWebsocketSuccess = "websocket" == _transport->getName();
  emit("open");
  flush();

  // we check for `readyState` in case an `open`
  // listener already closed the socket
  if (ReadyState::OPENED == _readyState && _upgrade && _transport->isSupportPaused()) {
    debug("starting upgrade probes");
    for (auto& upgrade : _upgrades)
    {
      probe(upgrade);
    }
  }
}

void EngineIOSocket::onPacket(const EngineIOPacket& packet)
{
  if (ReadyState::OPENING == _readyState || ReadyState::OPENED == _readyState ||
      ReadyState::CLOSING == _readyState) {
    debug("socket receive: type %s, data %s", packet.type.c_str(), packet.data.toString().c_str());

//cjh    emit("packet", packet);

    // Socket is live - any packet counts
    emit("heartbeat");

      if (packet.type == "open") {
        onHandshake(parsejson(packet.data.asString()));
      } else if (packet.type == "pong") {
        setPing();
        emit("pong");
      } else if (packet.type == "error") {
//        var err = new Error("server error");
//        err.code = packet.data;
//        this.onError(err);
      } else if (packet.type == "message") {
        emit("data", packet.data);
        emit("message", packet.data);
      }
  } else {
    debug("packet received with socket readyState %d", _readyState);
  }
}

void EngineIOSocket::onHandshake(const ValueObject& data)
{
    emit("handshake", data);
    _id = data.at("sid").asString();
    _transport->_query["sid"] = _id;
    _upgrades = filterUpgrades(data.at("upgrades").asArray());
    _pingInterval = data.at("pingInterval").asInt();
    _pingTimeout = data.at("pingTimeout").asInt();
    onOpen();
    // In case open handler closes socket
    if (ReadyState::CLOSED == _readyState)
        return;

    setPing();
    // Prolong liveness of socket on heartbeat
    off("heartbeat", _heartbeatId);
    on("heartbeat", std::bind(&EngineIOSocket::onHeartbeat, this, std::placeholders::_1), ID(&_heartbeatId));
}

void EngineIOSocket::onHeartbeat(const Value& timeout)
{
  clearTimeout(_pingTimeoutTimer);
  _pingTimeoutTimer = setTimeout([=] () {
    if (ReadyState::CLOSED == _readyState)
      return;
    onClose("ping timeout", "");
  }, timeout.getType() == Value::Type::INTEGER ? timeout.asInt() : (_pingInterval + _pingTimeout));
}

void EngineIOSocket::setPing()
{
  clearTimeout(_pingIntervalTimer);
  _pingIntervalTimer = setTimeout([=] () {
    debug("writing ping packet - expecting pong within %ldms", _pingTimeout);
    ping();
    onHeartbeat(Value((int)_pingTimeout));
  }, _pingInterval);
}

void EngineIOSocket::ping()
{
    sendPacket("ping", Value::NONE, ValueObject(), [this](const Value& unused) {
        emit("ping");
    });
}

void EngineIOSocket::onDrain()
{
  _writeBuffer.erase(_writeBuffer.begin(), _writeBuffer.begin() + _prevBufferLen);

  // setting prevBufferLen = 0 is very important
  // for example, when upgrading, upgrade packet is sent over,
  // and a nonzero prevBufferLen could cause problems on `drain`
  _prevBufferLen = 0;

  if (_writeBuffer.empty()) {
    emit("drain");
  } else {
    flush();
  }
}

void EngineIOSocket::flush()
{
  if (ReadyState::CLOSED != _readyState && _transport->isWritable() &&
    !_upgrading && !_writeBuffer.empty()) {
    debug("flushing %d packets in socket", (int)_writeBuffer.size());
    _transport->send(_writeBuffer);
    // keep track of current length of writeBuffer
    // splice writeBuffer and callbackBuffer on `drain`
    _prevBufferLen = _writeBuffer.size();
    emit("flush");
  }
}

// write
void EngineIOSocket::send(const Value& msg, const ValueObject& options, const std::function<void(const Value&)>& fn)
{
    sendPacket("message", msg, options, fn);
}

void EngineIOSocket::sendPacket(const std::string& type, const Value& data, const ValueObject& options, const std::function<void(const Value&)>& fn)
{
  if (ReadyState::CLOSING == _readyState || ReadyState::CLOSED == _readyState) {
    return;
  }

    EngineIOPacket packet;
    packet.type = type;
    packet.data = data;
    packet.options = options;

//cjh  emit("packetCreate", packet);
  _writeBuffer.push_back(packet);
  if (fn) once("flush", fn);
  flush();
}
//
void EngineIOSocket::close()
{
  auto closeTransport = [=](const Value&) {
      onClose("forced close", "");
      debug("socket closing - telling transport to close");
      _transport->close();
  };

  auto cleanupAndClose = [=](const Value&) {
      off("upgrade", _idCleanupAndCloseUpgrade);
      off("upgradeError", _idCleanupAndCloseUpgradeError);
      closeTransport(Value::NONE);
  };

  auto waitForUpgrade = [=]() {
    // wait for upgrade to finish since we can't send packets while pausing a transport
      once("upgrade", cleanupAndClose, ID(&_idCleanupAndCloseUpgrade));
      once("upgradeError", cleanupAndClose, ID(&_idCleanupAndCloseUpgradeError));
  };

  if (ReadyState::OPENING == _readyState || ReadyState::OPENED == _readyState) {
    _readyState = ReadyState::CLOSING;

    if (!_writeBuffer.empty()) {
      once("drain", [=](const Value&) {
        if (_upgrading) {
          waitForUpgrade();
        } else {
            closeTransport(Value::NONE);
        }
      });
    } else if (_upgrading) {
        waitForUpgrade();
    } else {
        closeTransport(Value::NONE);
    }
  }
}

void EngineIOSocket::onError(const std::string& err)
{
  debug("socket error %s", err.c_str());
  __priorWebsocketSuccess = false;
  emit("error", err);
  onClose("transport error", err);
}

void EngineIOSocket::onClose(const std::string& reason, const std::string& desc)
{
  if (ReadyState::OPENING == _readyState || ReadyState::OPENED == _readyState || ReadyState::CLOSING == _readyState) {
    debug("socket close with reason: %s", reason.c_str());

    // clear timers
    clearTimeout(_pingIntervalTimer);
    clearTimeout(_pingTimeoutTimer);

    // stop event from firing again for transport
    _transport->off("close");

    // ensure transport won't stay open
    _transport->close();

    // ignore further transport communication
    _transport->offAll();

    // set ready state
    _readyState = ReadyState::CLOSED;

    // clear session id
    _id = "";

    // emit close event
      ValueArray args;
      args.push_back(reason);
      args.push_back(desc);
      emit("close", args);

    // clean buffers after, so users can still
    // grab the buffers on `close` event
    _writeBuffer.clear();
    _prevBufferLen = 0;
  }
}

/**
 * Filters upgrades, returning only those matching client transports.
 *
 * @param {Array} server upgrades
 * @api private
 *
 */

std::vector<std::string> EngineIOSocket::filterUpgrades(const ValueArray& upgrades)
{
    std::vector<std::string> filteredUpgrades;
    for (size_t i = 0, j = upgrades.size(); i < j; i++) {
        if (std::find(_transports.begin(), _transports.end(), upgrades[i].asString()) != _transports.end())
            filteredUpgrades.push_back(upgrades[i].asString());
    }
    return filteredUpgrades;
}
