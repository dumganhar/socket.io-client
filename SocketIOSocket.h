#pragma once

class SocketIOSocket : public Emitter
{
public:

    /**
     * `Socket` constructor.
     *
     * @api public
     */

    SocketIOSocket(SocketIOManager* io, const std::string& nsp, const Opts& opts);

    /**
     * "Opens" the socket.
     *
     * @api public
     */

    void open();

    /**
     * Sends a `message` event.
     *
     * @return {Socket} self
     * @api public
     */

    void send();

    /**
     * Disconnects the socket manually.
     *
     * @return {Socket} self
     * @api public
     */
    void close(); // disconnect

    /**
     * Sets the compress flag.
     *
     * @param {Boolean} if `true`, compresses the sending data
     * @return {Socket} self
     * @api public
     */
    void compress(bool isCompress);

    virtual void emit(const std::string& eventName, const Args& args) override;

private:

    /**
     * Subscribe to open, close and packet events
     *
     * @api private
     */

    void subEvents();

    /**
     * Sends a packet.
     *
     * @param {Object} packet
     * @api private
     */

    void sendPacket(const Packet& packet);

    /**
     * Called upon engine `open`.
     *
     * @api private
     */
    void onopen();

    /**
     * Called upon engine `close`.
     *
     * @param {String} reason
     * @api private
     */

    void onclose(const std::string& reason);

    /**
     * Called with socket packet.
     *
     * @param {Object} packet
     * @api private
     */

    void onpacket(const Packet& packet);

    /**
     * Called upon a server event.
     *
     * @param {Object} packet
     * @api private
     */

    void onevent(const Packet& packet);

    /**
     * Produces an ack callback to emit with an event.
     *
     * @api private
     */

    void ack(int id);

    /**
     * Called upon a server acknowlegement.
     *
     * @param {Object} packet
     * @api private
     */

    void onack(const Packet& packet);

    /**
     * Called upon server connect.
     *
     * @api private
     */

    void onconnect();

    /**
     * Emit buffered events (received and emitted).
     *
     * @api private
     */

    void emitBuffered();

    /**
     * Called upon server disconnect.
     *
     * @api private
     */

    void ondisconnect();

    /**
     * Called upon forced client/server side disconnections,
     * this method ensures the manager stops tracking us and
     * that reconnections don't get triggered for this.
     *
     * @api private.
     */

    void destroy();


    SocketIOManager* _io;
    std::string _nsp;
    int _ids;
    std::unordered_map<int, std::function<void(SocketIOSocket*, const Packet&)>> _acks;
    std::vector<Data> _receiveBuffer;
    bool _connected;
};