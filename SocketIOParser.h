#pragma once

namespace socketio { namespace parser {

extern uint8_t protocol;

class Encoder
{
public:
    Encoder();
    ~Encoder();

    /**
     * Encode a packet as a single string if non-binary, or as a
     * buffer sequence, depending on packet type.
     *
     * @param {Object} obj - packet object
     * @param {Function} callback - function to handle encodings (likely engine.write)
     * @return Calls callback with Array of encodings
     * @api public
     */
    Data encode(const SocketIOPacket& obj);

private:

    /**
     * Encode packet as string.
     *
     * @param {Object} packet
     * @return {String} encoded
     * @api private
     */

    std::string encodeAsString(const SocketIOPacket& obj);

    /**
     * Encode packet as 'buffer sequence' by removing blobs, and
     * deconstructing packet into object with placeholders and
     * a list of buffers.
     *
     * @param {Object} packet
     * @return {Buffer} encoded
     * @api private
     */

    Data encodeAsBinary(const SocketIOPacket& obj);
};

class Decoder : public Emitter
{
public:
    Decoder();
    ~Decoder();

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