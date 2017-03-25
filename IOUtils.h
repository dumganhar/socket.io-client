#pragma once

#include <functional>
#include <sstream>

#include <stdint.h>

using ListenerId = uint64_t;

ListenerId grabListenerId(ListenerId* id = nullptr);

using TimerHandle = uint64_t;

TimerHandle setTimeout(const std::function<void()>& cb, long milliseconds);
void clearTimeout(TimerHandle);

template<class T>
std::string toString(T v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}
