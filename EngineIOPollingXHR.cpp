#include "EngineIOPollingXHR.h"
#include "EngineIORequest.h"
/**
 * XHR Polling constructor.
 *
 * @param {Object} opts
 * @api public
 */

EngineIOPollingXHR::EngineIOPollingXHR(const ValueObject& opts)
: EngineIOPolling(opts)
{
  _requestTimeout = opts.at("requestTimeout").asInt();

//  if (global.location) {
//    var isSSL = "https:" == location.protocol;
//    var port = location.port;
//
//    // some user agents have empty `location.port`
//    if (!port) {
//      port = isSSL ? 443 : 80;
//    }
//
//    this.xd = opts.hostname != global.location.hostname ||
//      port != opts.port;
//    this.xs = opts.secure != isSSL;
//  } else {
//    this.extraHeaders = opts.extraHeaders;
//  }
}

std::shared_ptr<EngineIORequest> EngineIOPollingXHR::request(const std::string& method, const Value& data)
{
//  opts.uri = this.uri();
//  opts.xd = this.xd;
//  opts.xs = this.xs;
//  opts.agent = this.agent || false;
//  opts.supportsBinary = this.supportsBinary;
//  opts.enablesXDR = this.enablesXDR;
//
//  // SSL options for Node.js client
//  opts.pfx = this.pfx;
//  opts.key = this.key;
//  opts.passphrase = this.passphrase;
//  opts.cert = this.cert;
//  opts.ca = this.ca;
//  opts.ciphers = this.ciphers;
//  opts.rejectUnauthorized = this.rejectUnauthorized;
//  opts.requestTimeout = this.requestTimeout;
//
//  // other options for Node.js client
//  opts.extraHeaders = this.extraHeaders;

    ValueObject opts;
    return std::make_shared<EngineIORequest>(opts);
};

/**
 * Sends data.
 *
 * @param {String} data to send.
 * @param {Function} called upon flush.
 * @api private
 */

void EngineIOPollingXHR::doWrite(const Value& data, const ValueFunction& fn)
{
  auto req = request("POST", data);
  req->on("success", fn);
  req->on("error", [this](const Value& err) {
    onError("xhr post error", err.asString());
  });
  _sendXhr = req;
}

void EngineIOPollingXHR::onPause()
{

}

/**
 * Starts a poll cycle.
 *
 * @api private
 */

void EngineIOPollingXHR::doPoll()
{
  debug("xhr poll");
  auto req = request("GET", Value::NONE);
  req->on("data", [this](const Value& data) {
    onData(data);
  });
  req->on("error", [this](const Value& err) {
    onError("xhr poll error", err.asString());
  });
  _pollXhr = req;
}





