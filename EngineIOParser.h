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


/**
 * Encodes multiple messages (payload).
 *
 *     <length>:data
 *
 * Example:
 *
 *     11:hello world2:hi
 *
 * If any contents are binary, they will be encoded as base64 strings. Base64
 * encoded strings are marked with a b before the length specifier
 *
 * @param {Array} packets
 * @api private
 */

Data encodePayload(const Packet& packet, bool supportsBinary);

/*
 * Decodes data when a payload is maybe expected. Possible binary contents are
 * decoded from their base64 representation
 *
 * @param {String} data, callback method
 * @api public
 */

Packet decodePayload(const Data& data, bool binaryType);

}} //namespace socketio { namespace parser {