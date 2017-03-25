WS::WS(const Opts& opts)
{
  var forceBase64 = (opts && opts.forceBase64);
  if (forceBase64) {
    _supportsBinary = false;
  }
  this.perMessageDeflate = opts.perMessageDeflate;
  this.usingBrowserWebSocket = BrowserWebSocket && !opts.forceNode;
  if (!this.usingBrowserWebSocket) {
    WebSocket = NodeWebSocket;
  }
  Transport.call(this, opts);
}

WS::~WS()
{
  
}

/**
 * Transport name.
 *
 * @api public
 */

const char* WS::getName() const
{
  return "websocket";
}

/**
 * Opens socket.
 *
 * @api private
 */

void WS::doOpen()
{
  if (!this.check()) {
    // let probe timeout
    return;
  }

  var uri = this.uri();
  var protocols = void (0);
  var opts = {
    agent: this.agent,
    perMessageDeflate: this.perMessageDeflate
  };

  // SSL options for Node.js client
  opts.pfx = this.pfx;
  opts.key = this.key;
  opts.passphrase = this.passphrase;
  opts.cert = this.cert;
  opts.ca = this.ca;
  opts.ciphers = this.ciphers;
  opts.rejectUnauthorized = this.rejectUnauthorized;
  if (this.extraHeaders) {
    opts.headers = this.extraHeaders;
  }
  if (this.localAddress) {
    opts.localAddress = this.localAddress;
  }

  _ws = new WebSocket(uri, protocols, opts);

  if (_ws == nullptr) {
    return this.emit("error", err);
  }

  _supportsBinary = true;

  addEventListeners();
}

void WS::addEventListeners()
{
  var self = this;

  _ws->onopen = function () {
    self.onOpen();
  };
  _ws->onclose = function () {
    self.onClose();
  };
  _ws->onmessage = function (ev) {
    self.onData(ev.data);
  };
  _ws->onerror = function (e) {
    self.onError('websocket error', e);
  };
}

bool WS::write(const std::vector<Packet>& packets)
{
  _writable = false;

  // encodePacket efficient as it uses WS framing
  // no need for encodePayload
  var total = packets.length;
  for (var i = 0, l = total; i < l; i++) {
    (function (packet) {
      parser.encodePacket(packet, _supportsBinary, [](Data& data) {
        if (!self.usingBrowserWebSocket) {
          // always create a new object (GH-437)
          var opts = {};
          if (packet.options) {
            opts.compress = packet.options.compress;
          }

          if (self.perMessageDeflate) {
            var len = 'string' == typeof data ? global.Buffer.byteLength(data) : data.length;
            if (len < self.perMessageDeflate.threshold) {
              opts.compress = false;
            }
          }
        }

        // Sometimes the websocket has already been closed but the browser didn't
        // have a chance of informing us about it yet, in that case send will
        // throw an error
        try {
          if (self.usingBrowserWebSocket) {
            // TypeError is thrown when passing the second argument on Safari
            _ws->send(data);
          } else {
            _ws->send(data, opts);
          }
        } catch (e) {
          debug("websocket closed before onclose event");
        }

        --total || done();
      });
    })(packets[i]);
  }

  function done () {
    emit('flush');

    // fake drain
    // defer to next tick to allow Socket to clear writeBuffer
    setTimeout(function () {
      _writable = true;
      emit('drain');
    }, 0);
  }
};

void WS::onClose()
{
    Transport::onClose();
}

void WS::doClose()
{
    _ws->close();
}

/**
 * Generates uri for connection.
 *
 * @api private
 */

WS.prototype.uri = function () {
  var query = this.query || {};
  var schema = this.secure ? 'wss' : 'ws';
  var port = '';

  // avoid port if default for schema
  if (this.port && (('wss' == schema && Number(this.port) != 443) ||
    ('ws' == schema && Number(this.port) != 80))) {
    port = ':' + this.port;
  }

  // append timestamp to URI
  if (this.timestampRequests) {
    query[this.timestampParam] = yeast();
  }

  // communicate binary support capabilities
  if (!_supportsBinary) {
    query.b64 = 1;
  }

  query = parseqs.encode(query);

  // prepend ? to query
  if (query.length) {
    query = '?' + query;
  }

  var ipv6 = this.hostname.indexOf(':') != -1;
  return schema + '://' + (ipv6 ? '[' + this.hostname + ']' : this.hostname) + port + this.path + query;
};

/**
 * Feature detection for WebSocket.
 *
 * @return {Boolean} whether this transport is available.
 * @api public
 */

WS.prototype.check = function () {
  return !!WebSocket && !('__initialize' in WebSocket && this.name == WS.prototype.name);
};
