#pragma once

#include "Emitter.h"

class EngineIORequest : public Emitter
{
public:
    EngineIORequest(const ValueObject& opts);
    virtual ~EngineIORequest();
private:

    void create();
};
