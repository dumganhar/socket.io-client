#include "EngineIOTransport.h"
#include "EngineIOParser.h"

//namespace socketio { namespace transport {

std::shared_ptr<EngineIOTransport> EngineIOTransport::create(const std::string& name, const ValueObject& opts)
{
    return nullptr;
}

EngineIOTransport::EngineIOTransport()
{

}

EngineIOTransport::~EngineIOTransport()
{

}

/**
 * Transport abstract constructor.
 *
 * @param {Object} options.
 * @api private
 */

bool EngineIOTransport::init (const Opts& opts)
{
  _path = opts.path;
  _hostname = opts.hostname;
  _port = opts.port;
  _secure = opts.secure;
  _query = opts.query;
//cjh  _timestampParam = opts.timestampParam;
//  _timestampRequests = opts.timestampRequests;
  _readyState = ReadyState::NONE;
//  _agent = opts.agent || false;
//  _socket = opts.socket;
//  _enablesXDR = opts.enablesXDR;

  // SSL options for Node.js client
//cjh  _pfx = opts.pfx;
//  _key = opts.key;
//  _passphrase = opts.passphrase;
//  _cert = opts.cert;
//  _ca = opts.ca;
//  _ciphers = opts.ciphers;
//  _rejectUnauthorized = opts.rejectUnauthorized;
//  _forceNode = opts.forceNode;

  // other options for Node.js client
//  _extraHeaders = opts.extraHeaders;
//  _localAddress = opts.localAddress;

    return true;
}

/**
 * Emits an error.
 *
 * @param {String} str
 * @return {Transport} for chaining
 * @api public
 */

void EngineIOTransport::onError(const std::string& msg, const std::string& desc)
{
//cjh  var err = new Error(msg);
//  err.type = "TransportError";
//  err.description = desc;
//  emit("error", err);
}

/**
 * Opens the transport.
 *
 * @api public
 */

bool EngineIOTransport::open()
{
    if (ReadyState::CLOSED == _readyState || ReadyState::NONE == _readyState) {
        _readyState = ReadyState::OPENING;
        return doOpen();
    }

    return false;
}

/**
 * Closes the transport.
 *
 * @api private
 */

void EngineIOTransport::close()
{
    if (ReadyState::OPENING == _readyState || ReadyState::OPENED == _readyState) {
        doClose();
        onClose();
    }
}

/**
 * Sends multiple packets.
 *
 * @param {Array} packets
 * @api private
 */

bool EngineIOTransport::send(const std::vector<EngineIOPacket>& packets)
{
    if (ReadyState::OPENED == _readyState) {
        return write(packets);
    }
    return false;
}

/**
 * Called upon open
 *
 * @api private
 */

void EngineIOTransport::onOpen()
{
    _readyState = ReadyState::OPENED;
    _writable = true;
    emit("open");
}

/**
 * Called with data.
 *
 * @param {String} data
 * @api private
 */

void EngineIOTransport::onData(const Value& data)
{
    EngineIOPacket packet = engineio::parser::decodePacket(data, true);
    onPacket(packet);
}

/**
 * Called with a decoded packet.
 */

void EngineIOTransport::onPacket(const EngineIOPacket& packet)
{
    emit("packet", packet);
}

/**
 * Called upon close.
 *
 * @api private
 */

void EngineIOTransport::onClose()
{
    _readyState = ReadyState::CLOSED;
    emit("close");
}

//}} // namespace socketio { namespace transport {
