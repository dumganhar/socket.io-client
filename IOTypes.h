#pragma once

class Buffer
{
public:
    Buffer();
    Buffer(uint8_t* data, size_t len);
    Buffer(const char* str);
    Buffer(const std::string& str);
    Buffer(const Buffer& o);
    Buffer(Buffer&& o);
    ~Buffer();

    Buffer& operator=(const char* str);
    Buffer& operator=(const std::string& str);
    Buffer& operator=(const Buffer& o);
    Buffer& operator=(Buffer&& o);
    uint8_t operator[](int index);

    bool isValid() const;
    const uint8_t* data() const;
    size_t length() const;
    const char* c_str() const;
    bool isBinary() const;

private:
    uint8_t* _data;
    size_t _len;
    bool _isBinary;
};

class Value;

using Array = std::vector<Value>;
using Object = std::unordered_map<std::string, Value>;

class Value
{
public:
    enum class Type
    {
        BUFFER,
        INTEGER,
        FLOAT,
        ARRAY,
        OBJECT
    };

    Type getType() const;

    Value();
    Value(const Value& o);
    Value(const Buffer& buf);
    Value(int intVal);
    Value(float floatVal);
    Value(const Array& arrVal);
    Value(const Object& objVal);
    ~Value();

    Value& operator=(const Value& o);
    Value& operator=(const Buffer& buf);
    Value& operator=(int intVal);
    Value& operator=(float floatVal);
    Value& operator=(const Array& arrVal);
    Value& operator=(const Object& objVal);

    const Buffer& asBuffer() const;
    int asInt() const;
    float asFloat() const;
    const Array& asArray() const;
    const Object& asObject() const;

private:
    union {
        Buffer* bufVal;
        int intVal;
        float floatVal;
        Array* arrVal;
        Object* objVal;
    } _u;

    Type _type;
};

using Values = std::vector<Value>;
using Args = Values;

class EngineIOPacket
{
public:
    static EngineIOPacket ERROR;
private:
    std::string _type;
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
        CONNECT: 0,

        /**
         * Packet type `disconnect`.
         */
        DISCONNECT: 1,

        /**
         * Packet type `event`.
         */
        EVENT: 2,

        /**
         * Packet type `ack`.
         */
        ACK: 3,

        /**
         * Packet type `error`.
         */
        ERROR: 4,

        /**
         * Packet type 'binary event'
         */
        BINARY_EVENT: 5,

        /**
         * Packet type `binary ack`. For acks with binary arguments.
         */
        BINARY_ACK: 6
    };

    SocketIOPacket();
    SocketIOPacket(const SocketIOPacket& packet);
    SocketIOPacket(SocketIOPacket&& packet);
    ~SocketIOPacket();

    SocketIOPacket& operator=(const SocketIOPacket& packet);
    SocketIOPacket& operator=(SocketIOPacket&& packet);

    bool isValid() const;
    void reset();

    void setId(const std::string& id);
    const std::string& getId() const;

    void setNsp(const std::string& nsp);
    const std::string& getNsp() const;

    void setType(PacketType type);
    PacketType getType() const;

    void setQuery(const std::string& query);
    const std::string& getQuery() const;

    void setData(const Values& data);
    void setData(Values&& data);
    const Values& getData() const;

    int getAttachments() const;

private:
    int _id;
    std::string _nsp;
    Type _type;
    std::string _query;
    int _attachments;  // -1 means not useful
    Values _data;
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




