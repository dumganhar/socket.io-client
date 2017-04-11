#pragma once

#include "EngineIOTransport.h"

class EngineIOPolling : public EngineIOTransport
{
public:

    EngineIOPolling(const ValueObject& opts);

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

    virtual void pause(const std::function<void()>& fn) override;

    /**
     * Writes a packets payload.
     *
     * @param {Array} data packets
     * @param {Function} drain callback
     * @api private
     */

    virtual bool write(const std::vector<EngineIOPacket>& packets) override;

    virtual const std::string& getName() const override;
    /**
     * Overloads onData to detect payloads.
     *
     * @api private
     */
    virtual void onData(const Value& data) override;

    virtual void onPause() = 0;

    /**
     * Opens the socket (triggers polling). We write a PING message to determine
     * when the transport is open.
     *
     * @api private
     */
    virtual bool doOpen() override;
    /**
     * For polling, send a close packet.
     *
     * @api private
     */
    virtual void doClose() override;

    virtual void doPoll() = 0;

    virtual void doWrite(const Value& data, const ValueFunction& fn) = 0;
private:

    bool _supportsBinary;
    bool _polling;

    ReadyState _readyState;
};
