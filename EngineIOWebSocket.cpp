#include "EngineIOWebSocket.h"
#include "IOUtils.h"
#include "EngineIOParser.h"

EngineIOWebSocket::EngineIOWebSocket(const ValueObject& opts)
: EngineIOTransport(opts)
{
  bool forceBase64 = opts.at("forceBase64").asBool();
  if (forceBase64) {
    _supportsBinary = false;
  }
  _perMessageDeflate = opts.at("perMessageDeflate").asBool();
}

EngineIOWebSocket::~EngineIOWebSocket()
{
  
}

/**
 * Transport name.
 *
 * @api public
 */

const std::string& EngineIOWebSocket::getName() const
{
    static const std::string name = "websocket";
    return name;
}

/**
 * Opens socket.
 *
 * @api private
 */

bool EngineIOWebSocket::doOpen()
{
//  if (!this.check()) {
//    // let probe timeout
//    return;
//  }

//  var uri = this.uri();
//  var protocols = void (0);
//  var opts = {
//    agent: this.agent,
//    perMessageDeflate: this.perMessageDeflate
//  };
//
//  // SSL options for Node.js client
//  opts.pfx = this.pfx;
//  opts.key = this.key;
//  opts.passphrase = this.passphrase;
//  opts.cert = this.cert;
//  opts.ca = this.ca;
//  opts.ciphers = this.ciphers;
//  opts.rejectUnauthorized = this.rejectUnauthorized;
//  if (this.extraHeaders) {
//    opts.headers = this.extraHeaders;
//  }
//  if (this.localAddress) {
//    opts.localAddress = this.localAddress;
//  }

//  _ws = new WebSocket(uri, protocols, opts);
//
//  if (_ws == nullptr) {
//    return this.emit("error", err);
//  }

  _supportsBinary = true;

  addEventListeners();

    return true;
}

void EngineIOWebSocket::addEventListeners()
{
//  _ws->onopen = function () {
//    self.onOpen();
//  };
//  _ws->onclose = function () {
//    self.onClose();
//  };
//  _ws->onmessage = function (ev) {
//    self.onData(ev.data);
//  };
//  _ws->onerror = function (e) {
//    self.onError("websocket error", e);
//  };
}

void EngineIOWebSocket::pause(const std::function<void()>& fn)
{
//cjh
}

bool EngineIOWebSocket::write(const std::vector<EngineIOPacket>& packets)
{
  _writable = false;

    auto done = [this]() {
        emit("flush");

        // fake drain
        // defer to next tick to allow Socket to clear writeBuffer
        setTimeout([this]() {
            _writable = true;
            emit("drain");
        }, 0);
    };

  // encodePacket efficient as it uses WS framing
  // no need for encodePayload
    for (const auto& packet : packets)
    {
        Value encodedPacket = engineio::parser::encodePacket(packet, _supportsBinary, false);
//        _ws->send(encodedPacket);
    }

    if (!packets.empty())
    {
        done();
    }

    return true;
}

void EngineIOWebSocket::onClose()
{
    EngineIOTransport::onClose();
}

void EngineIOWebSocket::doClose()
{
//    _ws->close();
}

/**
 * Generates uri for connection.
 *
 * @api private
 */

//EngineIOWebSocket.prototype.uri = function () {
//  var query = this.query || {};
//  var schema = this.secure ? "wss' : 'ws";
//  var port = "";
//
//  // avoid port if default for schema
//  if (this.port && (("wss" == schema && Number(this.port) != 443) ||
//    ("ws" == schema && Number(this.port) != 80))) {
//    port = ":" + this.port;
//  }
//
//  // append timestamp to URI
//  if (this.timestampRequests) {
//    query[this.timestampParam] = yeast();
//  }
//
//  // communicate binary support capabilities
//  if (!_supportsBinary) {
//    query.b64 = 1;
//  }
//
//  query = parseqs.encode(query);
//
//  // prepend ? to query
//  if (query.length) {
//    query = "?" + query;
//  }
//
//  var ipv6 = this.hostname.indexOf(":") != -1;
//  return schema + "://' + (ipv6 ? '[' + this.hostname + ']" : this.hostname) + port + this.path + query;
//};
//
///**
// * Feature detection for WebSocket.
// *
// * @return {Boolean} whether this transport is available.
// * @api public
// */
//
//EngineIOWebSocket.prototype.check = function () {
//  return !!WebSocket && !("__initialize" in WebSocket && this.name == WS.prototype.name);
//};
