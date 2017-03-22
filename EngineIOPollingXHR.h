#pragma once

struct Opts
{
    
};

class XHR : public Polling
{
public:
    XHR(const Opts& opts);

    virtual void doPoll();
    virtual void doWrite(const Data& data, const std::function<void()>& fn);

private:

    /**
     * Creates a request.
     *
     * @param {String} method
     * @api private
     */

    void request(const Opts& opts);
};