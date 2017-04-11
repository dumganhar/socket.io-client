#pragma once

#include "EngineIOTransport.h"

class IWebSocket;

class EngineIOWebSocket : public EngineIOTransport
{
public:
    EngineIOWebSocket(const ValueObject& opts);
    virtual ~EngineIOWebSocket();

    virtual const std::string& getName() const override;
    virtual void pause(const std::function<void()>& fn) override;
    virtual bool write(const std::vector<EngineIOPacket>& packets) override;
    virtual void onClose() override;
    virtual bool doOpen() override;
    virtual void doClose() override;

private:

    /**
     * Adds event listeners to the socket
     *
     * @api private
     */
    void addEventListeners();

    std::shared_ptr<IWebSocket> _ws;

    bool _supportsBinary;
    bool _perMessageDeflate;
};
