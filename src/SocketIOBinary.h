#pragma once

#include "IOTypes.h"

namespace binary {

class DeconstructedPacket
{
public:
    SocketIOPacket packet;
    ValueArray buffers;
};

/**
 * Replaces every Buffer | ArrayBuffer in packet with a numbered placeholder.
 * Anything with blobs or files should be fed through removeBlobs before coming
 * here.
 *
 * @param {Object} packet - socket.io event packet
 * @return {Object} with deconstructed packet and list of buffers
 * @api public
 */

DeconstructedPacket deconstructPacket(const SocketIOPacket& packet);


/**
 * Reconstructs a binary packet from its placeholder packet and buffers
 *
 * @param {Object} packet - event packet with placeholders
 * @param {Array} buffers - binary buffers to put in placeholder positions
 * @return {Object} reconstructed packet
 * @api public
 */

SocketIOPacket reconstructPacket(const SocketIOPacket& packet, const ValueArray& buffers);

} // namespace binary {
