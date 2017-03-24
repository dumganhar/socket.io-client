#pragma once

using ListenerId = uint64_t;

ListenerId grabListenerId(ListenerId* id = nullptr);

using TimerHandle = uint64_t;

TimerHandle setTimeout(const std::function<void()>& cb, long milliseconds);
void clearTimeout(TimerHandle);

bool hasBin(const Args& args);