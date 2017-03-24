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

    void send(const Args& args);

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
    void setCompress(bool compress);

    const std::string& getId() const { return _id; }

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

    void sendPacket(const SocketIOPacket& packet);

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

    void onpacket(const SocketIOPacket& packet);

    /**
     * Called upon a server event.
     *
     * @param {Object} packet
     * @api private
     */

    void onevent(const SocketIOPacket& packet);

    using AckCallback = std::function<void(const Data&)>;
    /**
     * Produces an ack callback to emit with an event.
     *
     * @api private
     */

    AckCallback ack(int id);

    /**
     * Called upon a server acknowlegement.
     *
     * @param {Object} packet
     * @api private
     */

    void onack(const SocketIOPacket& packet);

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

    void setId(const std::string& id) { _id = id; }

private:
    std::shared_ptr<SocketIOManager> _io;
    std::string _nsp;
    std::string _query;
    std::string _id; // An unique identifier for the socket session. Set after the connect event is triggered, and updated after the reconnect event.
    int _ids;
    std::unordered_map<int, AckCallback> _acks;
    std::vector<Args> _receiveBuffer;
    std::vector<SocketIOPacket> _sendBuffer;
    std::vector<OnObj> _subs;
    bool _connected;
    bool _disconnected;
    bool _compress;

    friend class SocketIOManager;
};