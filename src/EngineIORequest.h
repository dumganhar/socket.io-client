#pragma once

#include "Emitter.h"

class IHttpRequest;

class EngineIORequest : public Emitter
{
public:
    EngineIORequest(const ValueObject& opts);
    virtual ~EngineIORequest();
private:

    void create();

    std::shared_ptr<IHttpRequest> _xhr;
    std::string _method;
    std::string _uri;

    //  this.xd = !!opts.xd;
    Buffer _data;
    bool _isBinary;
    bool _supportsBinary;
    long _requestTimeout;

// SSL options for Node.js client
//  this.pfx = opts.pfx;
//  this.key = opts.key;
//  this.passphrase = opts.passphrase;
//  this.cert = opts.cert;
//  this.ca = opts.ca;
//  this.ciphers = opts.ciphers;
//  this.rejectUnauthorized = opts.rejectUnauthorized;
//
// other options for Node.js client
    std::unordered_map<std::string, std::string> _extraHeaders;
};
