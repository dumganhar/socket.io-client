#pragma once

class Request : public Emitter
{
public:
    Request(const Opts& opts);


private:

    void create();
};