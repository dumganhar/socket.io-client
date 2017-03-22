#pragma once

struct Data
{
    uint8_t* data;
    size_t len;
};

class Packet
{
public:
    std::string nsp;
    std::string type;
    std::string query;
    Data data;
};
