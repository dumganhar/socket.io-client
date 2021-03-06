#pragma once

#include "Emitter.h"

class SocketIOSocket;
class EngineIOSocket;

namespace socketio { namespace parser {
class Encoder;
class Decoder;

}} // namespace socketio { namespace parser {

class Backoff;

class SocketIOManager : public Emitter, public std::enable_shared_from_this<SocketIOManager>
{
public:
    SocketIOManager(const std::string& uri, const Opts& opts);

    ReadyState getReadyState() const { return _readyState; }

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

    std::shared_ptr<SocketIOSocket> createSocket(const std::string& nsp, const Opts& opts);

    /**
     * Called upon a socket close.
     *
     * @param {Socket} socket
     */

    void destroySocket(std::shared_ptr<SocketIOSocket> socket);

    /**
     * Sets the `reconnection` config.
     *
     * @param {Boolean} true/false if it should automatically reconnect
     * @return {Manager} self or value
     * @api public
     */

    void setAutoReconnect(bool autoReconnect);
    bool isAutoReconnect() const;

    /**
     * Sets the reconnection attempts config.
     *
     * @param {Number} max reconnection attempts before giving up
     * @return {Manager} self or value
     * @api public
     */

    void setReconnectionAttempts(int v);
    int getReconnectionAttempts() const;

    /**
     * Sets the delay between reconnections.
     *
     * @param {Number} delay
     * @return {Manager} self or value
     * @api public
     */

    void setReconnectionDelay(long v);
    long getReconnectionDelay() const;

    void setRandomizationFactor(float v);
    float getRandomizationFactor() const;

    /**
     * Sets the maximum delay between reconnections.
     *
     * @param {Number} delay
     * @return {Manager} self or value
     * @api public
     */

    void setReconnectionDelayMax(long v);
    long getReconnectionDelayMax() const;

    /**
     * Sets the connection timeout. `0` to disable
     *
     * @return {Manager} self or value
     * @api public
     */

    void setTimeoutDelay(long v);
    long getTimeoutDelay() const;

private:
    /**
     * Propagate given event to sockets and emit on `this`
     *
     * @api private
     */

    void emitAll(const std::string& eventName, const Value& args = Value::NONE);

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

    void onopen(const Value& unused);

    /**
     * Called upon a ping.
     *
     * @api private
     */

    void onping(const Value& unused);

    /**
     * Called upon a packet.
     *
     * @api private
     */

    void onpong(const Value& unused);

    /**
     * Called with data.
     *
     * @api private
     */

    void ondata(const Value& data);

    /**
     * Called when parser fully decodes a packet.
     *
     * @api private
     */

    void ondecoded(const Value& packet);

    /**
     * Called upon socket error.
     *
     * @api private
     */

    void onerror(const Value& err);

    /**
     * Writes a packet.
     *
     * @param {Object} packet
     * @api private
     */

    void sendPacket(SocketIOPacket& packet);

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

    void onclose(const Value& reason);

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

//
private:
    Opts _opts;
    std::unordered_map<std::string, std::shared_ptr<SocketIOSocket>> _nsps;
    std::vector<std::shared_ptr<SocketIOSocket>> _connecting;
    std::vector<OnObj> _subs;
    std::vector<SocketIOPacket> _packetBuffer;
    std::shared_ptr<EngineIOSocket> _engine;
    std::string _uri;

    bool _autoConnect;
    bool _reconnecting;
    bool _skipReconnect;
    bool _encoding;
    bool _reconnection;
    long _timeout; // ms
    int _reconnectionAttempts;
    long _reconnectionDelay;
    float _randomizationFactor;
    long _reconnectionDelayMax;

    ReadyState _readyState;

    std::shared_ptr<socketio::parser::Encoder> _encoder;
    std::shared_ptr<socketio::parser::Decoder> _decoder;

    std::shared_ptr<Backoff> _backoff;

    friend class SocketIOSocket;
};
