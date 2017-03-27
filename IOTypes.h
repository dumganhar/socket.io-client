#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <string>

#include <stdint.h>

class Buffer
{
public:
    Buffer();
    Buffer(const uint8_t* data, size_t len);
    Buffer(const char* str);
    Buffer(const std::string& str);
    Buffer(const Buffer& o);
    Buffer(Buffer&& o);
    ~Buffer();

    Buffer& operator=(const char* str);
    Buffer& operator=(const std::string& str);
    Buffer& operator=(const Buffer& o);
    Buffer& operator=(Buffer&& o);
    uint8_t operator[](int index) const;

    bool isValid() const;
    const uint8_t* data() const;
    size_t length() const;
    const char* c_str() const;
    bool isBinary() const;

    void setData(off_t offset, const uint8_t* data, size_t len);

    std::string toBase64String() const;

private:
    uint8_t* _data;
    size_t _len;
    bool _isBinary;
};

class Value;
class EngineIOPacket;
class SocketIOPacket;

using ValueArray = std::vector<Value>;
using ValueObject = std::unordered_map<std::string, Value>;
using ValueFunction = std::function<void(const Value&)>;

extern ValueObject OBJECT_NONE;

class Value
{
public:
    static Value NONE;

    enum class Type
    {
        NONE,
        STRING,
        BINARY,
        BOOLEAN,
        INTEGER,
        FLOAT,
        ARRAY,
        OBJECT,
        FUNCTION,
        ENGINEIO_PACKET,
        SOCKETIO_PACKET
    };

    Type getType() const;

    Value();
    Value(const Value& o);
    Value(const char* cstr);
    Value(const std::string& str);
    Value(const Buffer& buf);
    explicit Value(bool v);
    explicit Value(int intVal);
    explicit Value(float floatVal);
    Value(const ValueArray& arrVal);
    Value(const ValueObject& objVal);
    Value(const ValueFunction& func);
    Value(const EngineIOPacket& packet);
    Value(const SocketIOPacket& packet);

    ~Value();

    Value& operator=(const Value& o);
    Value& operator=(const char* o);
    Value& operator=(const std::string& o);
    Value& operator=(const Buffer& buf);
    Value& operator=(bool v);
    Value& operator=(int intVal);
    Value& operator=(float floatVal);
    Value& operator=(const ValueArray& arrVal);
    Value& operator=(const ValueObject& objVal);
    Value& operator=(const ValueFunction& func);
    Value& operator=(const EngineIOPacket& packet);
    Value& operator=(const SocketIOPacket& packet);

    const std::string& asString() const;
    const Buffer& asBuffer() const;
    bool asBool() const;
    int asInt() const;
    float asFloat() const;
    const ValueArray& asArray() const;
    const ValueObject& asObject() const;
    const EngineIOPacket& asEngineIOPacket() const;
    const SocketIOPacket& asSocketIOPacket() const;
    const ValueFunction& asFunction() const;

    bool isValid() const;
    bool hasBin() const;
    void reset();

    std::string toString() const;

    static ValueArray concat(const Value& a, const Value& b);

private:
    union {
        std::string* str;
        Buffer* buf;
        bool b;
        int i;
        float f;
        ValueArray* arr;
        ValueObject* obj;
        ValueFunction* func;
        EngineIOPacket* ep;
        SocketIOPacket* sp;
    } _u;

    Type _type;
};

using Args = ValueArray;

class EngineIOPacket
{
public:
    static EngineIOPacket NONE;
    static EngineIOPacket ERROR;

    bool isValid() const;

    std::string type;
    Value data;
    ValueObject options;
};

class SocketIOPacket
{
public:
    static SocketIOPacket ERROR;

    enum class Type
    {
        /**
         * Packet type `connect`.
         */
        CONNECT = 0,

        /**
         * Packet type `disconnect`.
         */
        DISCONNECT = 1,

        /**
         * Packet type `event`.
         */
        EVENT = 2,

        /**
         * Packet type `ack`.
         */
        ACK = 3,

        /**
         * Packet type `error`.
         */
        ERROR = 4,

        /**
         * Packet type "binary event"
         */
        BINARY_EVENT = 5,

        /**
         * Packet type `binary ack`. For acks with binary arguments.
         */
        BINARY_ACK = 6
    };

    SocketIOPacket();
    SocketIOPacket(const SocketIOPacket& packet);
    SocketIOPacket(SocketIOPacket&& packet);
    ~SocketIOPacket();

    SocketIOPacket& operator=(const SocketIOPacket& packet);
    SocketIOPacket& operator=(SocketIOPacket&& packet);

    bool isValid() const;
    void reset();

    std::string toString() const;

    int id;
    std::string nsp;
    Type type;
    ValueObject query;
    int attachments;  // -1 means not useful
    Value data;
    ValueObject options;
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
    ValueObject query;
    std::string path;// (String) name of the path that is captured on the server side (/socket.io)
    bool reconnection;// (Boolean) whether to reconnect automatically (true)
    int reconnectionAttempts;// (Number) number of reconnection attempts before giving up (Infinity)
    int reconnectionDelay;// (Number) how long to initially wait before attempting a new reconnection (1000). Affected by +/- randomizationFactor, for example the default initial delay will be between 500 to 1500ms.
    int reconnectionDelayMax;// (Number) maximum amount of time to wait between reconnections (5000). Each attempt increases the reconnection delay by 2x along with a randomization as above
    float randomizationFactor;// (Number) (0.5), 0 <= randomizationFactor <= 1
    int timeout;// (Number) connection timeout before a connect_error and connect_timeout events are emitted (20000)
    bool autoConnect;// (Boolean) by setting this false, you have to call manager.open whenever you decide it's appropriate
    bool secure;
    uint16_t port;
    std::string hostname;

    bool isValid() const;
};

using ListenerId = uint64_t;
using TimerHandle = int64_t;

extern TimerHandle INVALID_TIMER_HANDLE;

#define debug(...) printf(__VA_ARGS__)


