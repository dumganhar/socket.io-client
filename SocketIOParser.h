#pragma once

namespace socketio { namespace parser {

extern uint8_t protocol;

/**
 * Packet types.
 *
 * @api public
 */

extern const char* types;


enum class PacketType
{
    /**
     * Packet type `connect`.
     */
    CONNECT: 0,

    /**
     * Packet type `disconnect`.
     */
    DISCONNECT: 1,

    /**
     * Packet type `event`.
     */
    EVENT: 2,

    /**
     * Packet type `ack`.
     */
    ACK: 3,

    /**
     * Packet type `error`.
     */
    ERROR: 4,

    /**
     * Packet type 'binary event'
     */
    BINARY_EVENT: 5,

    /**
     * Packet type `binary ack`. For acks with binary arguments.
     */
    BINARY_ACK: 6
};

class Packet
{

};

class Encoder
{
public:
    std::string encode(const Packet& obj);
};

class Decoder
{
public:
    Packet decode(const std::string& str);
};

}} //namespace socketio { namespace parser {