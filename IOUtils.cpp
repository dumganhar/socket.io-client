#include "IOUtils.h"

ListenerId grabListenerId(ListenerId* id)
{
    static ListenerId __id = 0;
    *id = ++__id;
    return *id;
}