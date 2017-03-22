#pragma once

class SocketIOManager : public Emitter
{
public:
    SocketIOManager(const std::string& uri, const Opts& opts);

    /**
     * Sets the current transport `socket`.
     *
     * @param {Function} optional, callback
     * @return {Manager} self
     * @api public
     */

    void connect(const std::function<void()>& fn, const Opts& opts);

    /**
     * Creates a new socket for the given `nsp`.
     *
     * @return {Socket}
     * @api public
     */

    SocketIOSocket* createSocket(const std::string& nsp, const Opts& opts);

    /**
     * Called upon a socket close.
     *
     * @param {Socket} socket
     */

    void destroySocket(SocketIOSocket* socket);

    /**
     * Sets the `reconnection` config.
     *
     * @param {Boolean} true/false if it should automatically reconnect
     * @return {Manager} self or value
     * @api public
     */

    void reconnection(bool v);

    /**
     * Sets the reconnection attempts config.
     *
     * @param {Number} max reconnection attempts before giving up
     * @return {Manager} self or value
     * @api public
     */

    void reconnectionAttempts(int v);

    /**
     * Sets the delay between reconnections.
     *
     * @param {Number} delay
     * @return {Manager} self or value
     * @api public
     */

    void reconnectionDelay(float v);

    /**
     * Sets the maximum delay between reconnections.
     *
     * @param {Number} delay
     * @return {Manager} self or value
     * @api public
     */

    void reconnectionDelayMax(float v);

    /**
     * Sets the connection timeout. `false` to disable
     *
     * @return {Manager} self or value
     * @api public
     */

    void enableTimeout(bool v);
    bool isTimeoutEnabled() const;

private:
    /**
     * Propagate given event to sockets and emit on `this`
     *
     * @api private
     */

    void emitAll();

    /**
     * Starts trying to reconnect if reconnection is enabled and we have not
     * started reconnecting yet
     *
     * @api private
     */

    void maybeReconnectOnOpen();

    /**
     * Called upon transport open.
     *
     * @api private
     */

    void onopen();

    /**
     * Called upon a ping.
     *
     * @api private
     */

    void onping();

    /**
     * Called upon a packet.
     *
     * @api private
     */

    void onpong();

    /**
     * Called with data.
     *
     * @api private
     */

    void ondata(const Data& data);

    /**
     * Called when parser fully decodes a packet.
     *
     * @api private
     */

    void ondecoded(const Packet& packet);

    /**
     * Called upon socket error.
     *
     * @api private
     */

    void onerror(const std::string& err);

    /**
     * Writes a packet.
     *
     * @param {Object} packet
     * @api private
     */

    void sendPacket(const Packet& packet);

    /**
     * If packet buffer is non-empty, begins encoding the
     * next packet in line.
     *
     * @api private
     */

    void processPacketQueue();

    /**
     * Clean up transport subscriptions and packet buffer.
     *
     * @api private
     */

    void cleanup();

    /**
     * Close the current socket.
     *
     * @api private
     */

    // close
    void disconnect();

    /**
     * Called upon engine close.
     *
     * @api private
     */

    void onclose(const std::string& reason);

    /**
     * Attempt a reconnection.
     *
     * @api private
     */

    void reconnect();

    /**
     * Called upon successful reconnect.
     *
     * @api private
     */

    void onreconnect();

    /**
     * Update `socket.id` of all sockets
     *
     * @api private
     */

    void updateSocketIds();

    std::unordered_map<std::string, SocketIOSocket*> _nsps;
    std::vector<SocketIOSocket*> _connecting;
    bool _autoConnect;
    std::vector<OnObj> _subs;
    bool _timeout;
    std::shared_ptr<EngineIOSocket> _engine;
    std::string _uri;
    bool _reconnecting;
};