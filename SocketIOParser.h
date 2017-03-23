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

class Encoder
{
public:
    /**
     * Encode a packet as a single string if non-binary, or as a
     * buffer sequence, depending on packet type.
     *
     * @param {Object} obj - packet object
     * @param {Function} callback - function to handle encodings (likely engine.write)
     * @return Calls callback with Array of encodings
     * @api public
     */
    std::string encode(const Packet& obj);

private:

    /**
     * Encode packet as string.
     *
     * @param {Object} packet
     * @return {String} encoded
     * @api private
     */

    std::string encodeAsString(const Packet& obj);

};

class Decoder : public Emitter
{
public:
    Decoder();
    virtual ~Decoder();

    /**
     * Decodes an ecoded packet string into packet JSON.
     *
     * @param {String} obj - encoded packet
     * @return {Object} packet
     * @api public
     */
    void add(const Data& obj);

    /**
     * Deallocates a parser's resources
     *
     * @api public
     */

    void destroy();

private:

    /**
     * Decode a packet String (JSON data)
     *
     * @param {String} str
     * @return {Object} packet
     * @api private
     */
    Packet decodeString(const std::string& str);
};

}} //namespace socketio { namespace parser {