#pragma once

class WS : public Transport
{
public:
    WS(const Opts& opts);
    virtual ~WS();

    virtual const char* getTransportName() const override;
    virtual bool write(const std::vector<Packet>& packets) override;
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

    WebSocket* _ws;
};