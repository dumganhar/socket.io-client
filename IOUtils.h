#pragma once

#include "IOTypes.h"

#include <functional>
#include <sstream>
#include <stdint.h>

ListenerId grabListenerId(ListenerId* id = nullptr);

#define ID grabListenerId

TimerHandle setTimeout(const std::function<void()>& cb, long milliseconds);
void clearTimeout(TimerHandle);

template<class T>
std::string toString(T v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

std::string utf8Encode(const std::string& str);
std::string utf8Decode(const std::string& str);

std::string base64Encode(const Buffer& buf);
Buffer base64Decode(const std::string& str);

ValueObject parsejson(const std::string& str);

std::string queryToString(const ValueObject& obj);

class IHttpRequest
{
public:
    virtual ~IHttpRequest() {}

    virtual bool open(const std::string& method, const std::string& uri, const std::string& caFilePath) = 0;
    virtual void setRequestHeader(const std::string& key, const std::string& value) = 0;
    virtual void send(const Buffer& data) = 0;
    virtual void abort() = 0;

    std::function<void()> onload;
    std::function<void()> onerror;
    std::function<void()> onreadystatechange;

    long timeout;
    int status;
    Buffer response;
    std::string responseType;
    std::unordered_map<std::string, std::string> responseHeaders;
};

class IHttpRequestFactory
{
public:
    virtual ~IHttpRequestFactory() {}
    virtual std::shared_ptr<IHttpRequest> create() = 0;
};

class IWebSocket
{
public:
    IWebSocket()
    : onopen(nullptr)
    , onmessage(nullptr)
    , onclose(nullptr)
    , onerror(nullptr)
    {}
    virtual ~IWebSocket() {}

    virtual bool open(const std::string& uri, const std::vector<std::string>& protocols, const std::string& caFilePath) = 0;
    virtual void close() = 0;
    virtual void send(const Buffer& data) = 0;

    std::function<void()> onopen;
    std::function<void(const Buffer&)> onmessage;
    std::function<void(const std::string&)> onclose;
    std::function<void(const std::string&)> onerror;

    long timeout;
};

class IWebSocketFactory
{
public:
    virtual ~IWebSocketFactory() {}
    virtual std::shared_ptr<IWebSocket> create() = 0;
};

void setHttpRequestFactory(std::shared_ptr<IHttpRequestFactory> factory);
std::shared_ptr<IHttpRequestFactory> getHttpRequestFactor();

void setWebSocketFactory(std::shared_ptr<IWebSocketFactory> factory);
std::shared_ptr<IWebSocketFactory> getWebSocketFactory();
