#pragma once

#include "EngineIOPolling.h"

class EngineIORequest;

class EngineIOPollingXHR : public EngineIOPolling
{
public:
    EngineIOPollingXHR(const ValueObject& opts);

    virtual void doPoll() override;
    virtual void doWrite(const Value& data, const ValueFunction& fn) override;
    virtual void onPause() override;
private:

    /**
     * Creates a request.
     *
     * @param {String} method
     * @api private
     */

    std::shared_ptr<EngineIORequest> request(const std::string& method, const Value& data);

    long _requestTimeout;

    std::shared_ptr<EngineIORequest> _pollXhr;
    std::shared_ptr<EngineIORequest> _sendXhr;
};
