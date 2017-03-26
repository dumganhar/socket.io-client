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
