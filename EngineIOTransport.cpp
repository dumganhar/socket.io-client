#include "EngineIOTransport.h"

namespace socketio { namespace transport {

/**
 * Transport abstract constructor.
 *
 * @param {Object} options.
 * @api private
 */

bool Transport::init (const Opts& opts)
{
  _path = opts.path;
  _hostname = opts.hostname;
  _port = opts.port;
  _secure = opts.secure;
  _query = opts.query;
  _timestampParam = opts.timestampParam;
  _timestampRequests = opts.timestampRequests;
  _readyState = ReadyState::NONE;
  _agent = opts.agent || false;
  _socket = opts.socket;
  _enablesXDR = opts.enablesXDR;

  // SSL options for Node.js client
  _pfx = opts.pfx;
  _key = opts.key;
  _passphrase = opts.passphrase;
  _cert = opts.cert;
  _ca = opts.ca;
  _ciphers = opts.ciphers;
  _rejectUnauthorized = opts.rejectUnauthorized;
  _forceNode = opts.forceNode;

  // other options for Node.js client
  _extraHeaders = opts.extraHeaders;
  _localAddress = opts.localAddress;
}

/**
 * Emits an error.
 *
 * @param {String} str
 * @return {Transport} for chaining
 * @api public
 */

void Transport::onError(const std::string& msg, const std::string& desc)
{
  var err = new Error(msg);
  err.type = 'TransportError';
  err.description = desc;
  emit('error', err);
}

/**
 * Opens the transport.
 *
 * @api public
 */

bool Transport::open()
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

void Transport::close()
{
  if (ReadyState::OPENING == _readyState || ReadyState::OPEN == _readyState) {
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

bool Transport::send(const std::vector<Packet>& packets)
{
  if (ReadyState::OPEN == _readyState) {
      return write(packets);
  }
  return false;
}

/**
 * Called upon open
 *
 * @api private
 */

void Transport::onOpen()
{
  _readyState = ReadyState::OPEN;
  _writable = true;
  emit("open");
}

/**
 * Called with data.
 *
 * @param {String} data
 * @api private
 */

void Transport::onData(const Data& data)
{
  Packet packet = parser::decodePacket(data, _socket.binaryType);
  onPacket(packet);
}

/**
 * Called with a decoded packet.
 */

void Transport::onPacket(const Packet& packet)
{
  emit("packet", packet);
}

/**
 * Called upon close.
 *
 * @api private
 */

void Transport::onClose()
{
  _readyState = ReadyState::CLOSED;
  emit("close");
}

}} // namespace socketio { namespace transport {
