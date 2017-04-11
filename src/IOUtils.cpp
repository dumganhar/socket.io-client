#include "IOUtils.h"

ListenerId grabListenerId(ListenerId* id)
{
    static ListenerId __id = 0;
    ListenerId ret = ++__id;
    if (id != nullptr)
        *id = ret;
    return ret;
}

TimerHandle setTimeout(const std::function<void()>& cb, long milliseconds)
{
    return 0;
}

void clearTimeout(TimerHandle)
{

}

std::string utf8Encode(const std::string& str)
{
    return str;
}

std::string utf8Decode(const std::string& str)
{
    return str;
}

std::string base64Encode(const Buffer& buf)
{
    return "";
}

Buffer base64Decode(const std::string& str)
{
    return Buffer(nullptr, 0);
}

ValueObject parsejson(const std::string& str)
{
    return ValueObject();
}

std::string queryToString(const ValueObject& obj)
{
    return "";
}

///

static std::shared_ptr<IHttpRequestFactory> __httpFactory;
static std::shared_ptr<IWebSocketFactory> __wsFactory;

void setHttpRequestFactory(std::shared_ptr<IHttpRequestFactory> factory)
{
    __httpFactory = factory;
}

std::shared_ptr<IHttpRequestFactory> getHttpRequestFactor()
{
    return __httpFactory;
}

void setWebSocketFactory(std::shared_ptr<IWebSocketFactory> factory)
{
    __wsFactory = factory;
}

std::shared_ptr<IWebSocketFactory> getWebSocketFactory()
{
    return __wsFactory;
}

