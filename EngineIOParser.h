#pragma once

#include <unordered_map>
#include <string>

namespace socketio { namespace parser {

/**
 * Current protocol version.
 */
extern uint8_t protocol;



/**
 * Encodes a packet.
 *
 *     <packet type id> [ <data> ]
 *
 * Example:
 *
 *     5hello world
 *     3
 *     4
 *
 * Binary is encoded in an identical principle
 *
 * @api private
 */

Data encodePacket(const Packet& packet, bool supportsBinary, bool utf8encode);

/**
 * Decodes a packet. Data also available as an ArrayBuffer if requested.
 *
 * @return {Object} with `type` and `data` (if any)
 * @api private
 */

Packet decodePacket(const Data& data, bool binaryType, bool utf8decode);

}} //namespace socketio { namespace parser {