#pragma once

struct Opts
{
    
};

class Polling : public Transport
{
public:

    Polling(const Opts& opts);

    /**
     * Starts polling cycle.
     *
     * @api public
     */

    void poll();

    /**
     * Pauses polling.
     *
     * @param {Function} callback upon buffers are flushed and transport is paused
     * @api private
     */

    void pause();

    /**
     * Writes a packets payload.
     *
     * @param {Array} data packets
     * @param {Function} drain callback
     * @api private
     */

    virtual bool write(const std::vector<Packet>& packets);

    virtual const char* getTransportName() const override;
    /**
     * Overloads onData to detect payloads.
     *
     * @api private
     */
    virtual void onData(const Data& data);

    /**
     * Opens the socket (triggers polling). We write a PING message to determine
     * when the transport is open.
     *
     * @api private
     */
    virtual bool doOpen();
    /**
     * For polling, send a close packet.
     *
     * @api private
     */
    virtual void doClose();

    virtual void doPoll() = 0;

    virtual void doWrite(const Data& data, const std::function<void()>& fn) = 0;
private:

    bool _supportsBinary;
    bool _polling;
};