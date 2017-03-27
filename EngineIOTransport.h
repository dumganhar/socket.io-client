#pragma once

#include "Emitter.h"

//namespace socketio { namespace transport {

class EngineIOTransport : public Emitter
{
public:

    static std::shared_ptr<EngineIOTransport> create(const std::string& name, const ValueObject& opts);

    virtual ~EngineIOTransport();

    bool open();
    void close();
    bool send(const std::vector<EngineIOPacket>& packets);
    virtual void pause(const std::function<void()>& fn) = 0;

    bool isWritable() const { return _writable; }

    virtual void onOpen();
    virtual void onClose();
    virtual void onError(const std::string& msg, const std::string& desc);
    virtual void onData(const Value& data);
    virtual void onPacket(const EngineIOPacket& packet);

    /**
     * Writes a packets payload.
     *
     * @param {Array} data packets
     * @param {Function} drain callback
     * @api private
     */
    virtual bool write(const std::vector<EngineIOPacket>& packets) = 0;
    virtual bool doOpen() = 0;
    virtual void doClose() = 0;
    virtual const std::string& getName() const = 0;

protected:
    EngineIOTransport(const ValueObject& opts);

    std::string _path;
    std::string _hostname;
    uint16_t _port;
    bool _secure;
    ValueObject _query;
    // _timestampParam;
    // _timestampRequests;
    ReadyState _readyState;
    // _agent;
    // _socket;
    bool _enablesXDR;

    // SSL options for Node.js client
    std::string _pfx;
    std::string _key;
    std::string _passphrase;
    std::string _cert;
    std::string _ca;
    std::string _ciphers;
    bool _rejectUnauthorized;
    bool _forceNode;

    // other options for Node.js client
    std::string _extraHeaders;
    std::string _localAddress;

    bool _writable;

    friend class EngineIOSocket;
};

//}} // namespace socketio { namespace transport {
