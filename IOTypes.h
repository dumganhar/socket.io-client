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


enum class ReadyState
{
    NONE,
    OPENING,
    OPENED,
    CLOSING,
    CLOSED
};

struct Opts
{
    bool multiplex; // reuse an existing Manager for subsequent calls, unless the multiplex option is passed with false
    bool forceNew; // force new connection
    std::string query;
    std::string path;// (String) name of the path that is captured on the server side (/socket.io)
    bool reconnection;// (Boolean) whether to reconnect automatically (true)
    int reconnectionAttempts;// (Number) number of reconnection attempts before giving up (Infinity)
    int reconnectionDelay;// (Number) how long to initially wait before attempting a new reconnection (1000). Affected by +/- randomizationFactor, for example the default initial delay will be between 500 to 1500ms.
    int reconnectionDelayMax;// (Number) maximum amount of time to wait between reconnections (5000). Each attempt increases the reconnection delay by 2x along with a randomization as above
    float randomizationFactor;// (Number) (0.5), 0 <= randomizationFactor <= 1
    int timeout;// (Number) connection timeout before a connect_error and connect_timeout events are emitted (20000)
    bool autoConnect;// (Boolean) by setting this false, you have to call manager.open whenever you decide it's appropriate
};