#pragma once

class EngineIOSocket : public Emitter
{
public:
    /**
     * Socket constructor.
     *
     * @param {String|Object} uri or options
     * @param {Object} options
     * @api public
     */
    EngineIOSocket(const std::string& uri, const Opts& opts);

    /**
     * Protocol version.
     *
     * @api public
     */
    int getProtocolVersion() const;

    /**
     * Sends a message.
     *
     * @param {String} message.
     * @param {Function} callback function.
     * @param {Object} options.
     * @return {Socket} for chaining.
     * @api public
     */
    void send(const Value& msg, const Opts& options, const std::function<void()>& fn);

    const std::string& getId() const { return _id; }

    /**
     * Called when connection is deemed open.
     *
     * @api public
     */
    void onOpen();

private:

    /**
     * Initializes transport to use and starts probe.
     *
     * @api private
     */
    void open();

    /**
     * Sets the current transport. Disables the existing one (if any).
     *
     * @api private
     */

    void setTransport(std::shared_ptr<Transport> transport);

    /**
     * Creates transport of the given type.
     *
     * @param {String} transport name
     * @return {Transport}
     * @api private
     */

    std::shared_ptr<Transport> createTransport(const std::string& name);

    /**
     * Probes a transport.
     *
     * @param {String} transport name
     * @api private
     */

    void probe(const std::string& name);

    /**
     * Flush write buffers.
     *
     * @api private
     */
    void flush();

    /**
     * Sends a packet.
     *
     * @param {String} packet type.
     * @param {String} data.
     * @param {Object} options.
     * @param {Function} callback function.
     * @api private
     */
    void sendPacket(const std::string& type, const Value& data, const Opts& options, const std::function<void()>& fn);

    /**
     * Closes the connection.
     *
     * @api private
     */
    void close();
    /**
     * Called upon transport close.
     *
     * @api private
     */

    void onClose(const std::string& reason, const std::string& desc);
    void onError(const std::string& err);

    /**
     * Called on `drain` event
     *
     * @api private
     */

    void onDrain();

    /**
     * Handles a packet.
     *
     * @api private
     */

    void onPacket(const Packet& packet);

    /**
     * Called upon handshake completion.
     *
     * @param {Object} handshake obj
     * @api private
     */

    void onHandshake(const Data& data);

    /**
     * Resets ping timeout.
     *
     * @api private
     */

    void onHeartbeat(float timeout);

    /**
     * Pings server every `this.pingInterval` and expects response
     * within `this.pingTimeout` or closes connection.
     *
     * @api private
     */

    void setPing();

    /**
    * Sends a ping packet.
    *
    * @api private
    */

    void ping();

    /**
     * Filters upgrades, returning only those matching client transports.
     *
     * @param {Array} server upgrades
     * @api private
     *
     */

    void filterUpgrades(const std::vector<std::string>& upgrades);

    std::string _id;
    std::vector<std::string> _transports;
    std::vector<std::string> _upgrades;

    std::vector<Packet> _writeBuffer;

    std::shared_ptr<Transport> _transport;
    ReadyState _readyState;
    bool _rememberUpgrade;
    bool _upgrading;
    bool _upgrade;

    float _pingInterval;
    float _pingTimeout;

    ListenerId _heartbeatId;

    size_t _prevBufferLen;

    // Listener Ids
    ListenerId _idOnTransportOpen;
    ListenerId _idOnerror;
    ListenerId _idOnTransportClose;
    ListenerId _idOnclose;
    ListenerId _idOnupgrade;
    ListenerId _idCleanupAndCloseUpgrade;
    ListenerId _idCleanupAndCloseUpgradeError;
};