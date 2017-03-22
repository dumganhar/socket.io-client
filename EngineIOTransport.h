#pragma once

#include <string>

namespace socketio { namespace transport {

class Transport : public Emitter
{
public:
    virtual ~Transport();

    bool init(const Opts& opts);

    bool open();
    void close();
    bool send(const Packet& packet);

    virtual void onOpen();
    virtual void onClose();
    virtual void onError(const std::string& msg, const std::string& desc);
    virtual void onData(const Data& data);
    virtual void onPacket(const Packet& packet);

    virtual const char* getTransportName() = 0;
    /**
     * Writes a packets payload.
     *
     * @param {Array} data packets
     * @param {Function} drain callback
     * @api private
     */
    virtual bool write(const std::vector<Packet>& packets) = 0;
    virtual bool doOpen() = 0;
    virtual void doClose() = 0;
    virtual const std::string& getTransportName() const = 0;

private:

    enum class ReadyState
    {
        NONE,
        OPENING,
        OPENED,
        CLOSED
    };

    std::string _path;
    std::string _hostname;
    uint16_t _port;
    bool _secure;
    std::string _query;
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
}

}} // namespace socketio { namespace transport {
