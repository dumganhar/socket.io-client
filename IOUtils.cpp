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

bool hasBin(const Args& args)
{
    return false;
}