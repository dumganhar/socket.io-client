#include "IOTypes.h"

#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Buffer::Buffer()
: _data(nullptr)
, _len(0)
, _isBinary(false)
{

}

Buffer::Buffer(const uint8_t* data, size_t len)
: _len(len)
, _isBinary(true)
{
    if (_len > 0)
    {
        _data = (uint8_t*) malloc(_len);
        if (data) {
            memcpy(_data, data, _len);
        } else {
            memset(_data, 0, _len);
        }
    }
}

Buffer::Buffer(const char* str)
: _isBinary(false)
{
    _len = strlen(str);
    _data = (uint8_t*) malloc(_len + 1);
    _data[_len] = '\0';
    strcpy((char*)_data, str);
}

Buffer::Buffer(const std::string& str)
: _isBinary(false)
{
    _len = str.length();
    _data = (uint8_t*) malloc(_len + 1);
    _data[_len] = '\0';
    strcpy((char*)_data, str.c_str());
}

Buffer::Buffer(const Buffer& o)
{
    _isBinary = o._isBinary;
    _len = o._len;
    _data = (uint8_t*) malloc(_len);
    memcpy(_data, o._data, _len);
}

Buffer::Buffer(Buffer&& o)
{
    _isBinary = o._isBinary;
    _len = o._len;
    _data = o._data;
    o._len = 0;
    o._data = nullptr;
    o._isBinary = false;
}

Buffer::~Buffer()
{
    free(_data);
}

Buffer& Buffer::operator=(const char* str)
{
    _isBinary = false;
    _len = strlen(str);
    _data = (uint8_t*) malloc(_len + 1);
    _data[_len] = '\0';
    strcpy((char*)_data, str);
    return *this;
}

Buffer& Buffer::operator=(const std::string& str)
{
    _isBinary = false;
    _len = str.length();
    _data = (uint8_t*) malloc(_len + 1);
    _data[_len] = '\0';
    strcpy((char*)_data, str.c_str());
    return *this;
}

Buffer& Buffer::operator=(const Buffer& o)
{
    if (this != &o)
    {
        _isBinary = o._isBinary;
        _len = o._len;
        _data = (uint8_t*) malloc(_len);
        memcpy(_data, o._data, _len);
    }
    return *this;
}

Buffer& Buffer::operator=(Buffer&& o)
{
    if (this != &o)
    {
        _isBinary = o._isBinary;
        _len = o._len;
        _data = o._data;
        o._len = 0;
        o._data = nullptr;
        o._isBinary = false;
    }
    return *this;
}

uint8_t Buffer::operator[](int index) const
{
    return _data[index];
}

bool Buffer::isValid() const
{
    return _data != nullptr && _len > 0;
}

const uint8_t* Buffer::data() const
{
    return _data;
}

size_t Buffer::length() const
{
    return _len;
}

const char* Buffer::c_str() const
{
    return (const char*) _data;
}

bool Buffer::isBinary() const
{
    return _isBinary;
}

std::string Buffer::toBase64String() const
{
    return "";
}

void Buffer::setData(off_t offset, const uint8_t* data, size_t len)
{
    if (offset + len > _len)
    {
        return;
    }

    memcpy(_data + offset, data, len);
}

///

ValueObject OBJECT_NONE;

Value Value::NONE = Value();

Value::Type Value::getType() const
{
    return _type;
}

Value::Value()
: _type(Type::NONE)
{
    memset(&_u, 0, sizeof(_u));
}

Value::Value(const Value& o)
{
    _type = o._type;
    switch (_type)
    {
        case Type::STRING:
            _u.str = new std::string(*o._u.str);
            break;
        case Type::BINARY:
            _u.buf = new Buffer(*o._u.buf);
            break;
        case Type::BOOLEAN:
            _u.b = o._u.b;
            break;
        case Type::INTEGER:
            _u.i = o._u.i;
            break;
        case Type::FLOAT:
            _u.f = o._u.f;
            break;
        case Type::ARRAY:
            _u.arr = new ValueArray(*o._u.arr);
            break;
        case Type::OBJECT:
            _u.obj = new ValueObject(*o._u.obj);
            break;
        case Type::FUNCTION:
            _u.func = new ValueFunction(*o._u.func);
            break;
        case Type::ENGINEIO_PACKET:
            _u.ep = new EngineIOPacket(*o._u.ep);
            break;
        case Type::SOCKETIO_PACKET:
            _u.sp = new SocketIOPacket(*o._u.sp);
            break;
        default:
            break;
    }
}

Value::Value(const char* str)
{
    _type = Type::STRING;
    _u.str = new std::string(str);
}

Value::Value(const std::string& str)
{
    _type = Type::STRING;
    _u.str = new std::string(str);
}

Value::Value(const Buffer& buf)
{
    _type = Type::BINARY;
    _u.buf = new Buffer(buf);
}

Value::Value(bool v)
{
    _type = Type::BOOLEAN;
    _u.b = v;
}

Value::Value(int intVal)
{
    _type = Type::INTEGER;
    _u.i = intVal;
}

Value::Value(float floatVal)
{
    _type = Type::FLOAT;
    _u.i = floatVal;
}

Value::Value(const ValueArray& arrVal)
{
    _type = Type::ARRAY;
    _u.arr = new ValueArray(arrVal);
}

Value::Value(const ValueObject& objVal)
{
    _type = Type::OBJECT;
    _u.obj = new ValueObject(objVal);
}

Value::Value(const EngineIOPacket& packet)
{
    _type = Type::ENGINEIO_PACKET;
    _u.ep = new EngineIOPacket(packet);
}

Value::Value(const SocketIOPacket& packet)
{
    _type = Type::SOCKETIO_PACKET;
    _u.sp = new SocketIOPacket(packet);
}

Value::Value(const ValueFunction& func)
{
    _type = Type::FUNCTION;
    _u.func = new ValueFunction(func);
}

Value::~Value()
{
    reset();
}

Value& Value::operator=(const Value& o)
{
    if (this != &o)
    {
        _type = o._type;
        switch (_type)
        {
            case Type::STRING:
                _u.str = new std::string(*o._u.str);
                break;
            case Type::BINARY:
                _u.buf = new Buffer(*o._u.buf);
                break;
            case Type::BOOLEAN:
                _u.b = o._u.b;
                break;
            case Type::INTEGER:
                _u.i = o._u.i;
                break;
            case Type::FLOAT:
                _u.f = o._u.f;
                break;
            case Type::ARRAY:
                _u.arr = new ValueArray(*o._u.arr);
                break;
            case Type::OBJECT:
                _u.obj = new ValueObject(*o._u.obj);
                break;
            case Type::FUNCTION:
                _u.func = new ValueFunction(*o._u.func);
                break;
            case Type::ENGINEIO_PACKET:
                _u.ep = new EngineIOPacket(*o._u.ep);
                break;
            case Type::SOCKETIO_PACKET:
                _u.sp = new SocketIOPacket(*o._u.sp);
                break;
            default:
                break;
        }
    }
    return *this;
}

Value& Value::operator=(const char* str)
{
    if (_type != Type::STRING)
        reset();
    _type = Type::STRING;
    _u.str = new std::string(str);
    return *this;
}

Value& Value::operator=(const std::string& str)
{
    if (_type != Type::STRING)
        reset();
    _type = Type::STRING;
    _u.str = new std::string(str);
    return *this;
}

Value& Value::operator=(const Buffer& buf)
{
    if (_type != Type::BINARY)
        reset();
    _type = Type::BINARY;
    _u.buf = new Buffer(buf);
    return *this;
}

Value& Value::operator=(bool v)
{
    if (_type != Type::BOOLEAN)
        reset();
    _type = Type::BOOLEAN;
    _u.b = v;
    return *this;
}

Value& Value::operator=(int intVal)
{
    if (_type != Type::INTEGER)
        reset();
    _type = Type::INTEGER;
    _u.i = intVal;
    return *this;
}

Value& Value::operator=(float floatVal)
{
    if (_type != Type::FLOAT)
        reset();
    _type = Type::FLOAT;
    _u.f = floatVal;
    return *this;
}

Value& Value::operator=(const ValueArray& arrVal)
{
    if (_type != Type::ARRAY)
        reset();
    _type = Type::ARRAY;
    _u.arr = new ValueArray(arrVal);
    return *this;
}

Value& Value::operator=(const ValueObject& objVal)
{
    if (_type != Type::OBJECT)
        reset();
    _type = Type::OBJECT;
    _u.obj = new ValueObject(objVal);
    return *this;
}

Value& Value::operator=(const EngineIOPacket& packet)
{
    if (_type != Type::ENGINEIO_PACKET)
        reset();
    _type = Type::ENGINEIO_PACKET;
    _u.ep = new EngineIOPacket(packet);
    return *this;
}

Value& Value::operator=(const SocketIOPacket& packet)
{
    if (_type != Type::SOCKETIO_PACKET)
        reset();
    _type = Type::SOCKETIO_PACKET;
    _u.sp = new SocketIOPacket(packet);
    return *this;
}

Value& Value::operator=(const ValueFunction& func)
{
    if (_type != Type::FUNCTION)
        reset();
    _type = Type::FUNCTION;
    _u.func = new ValueFunction(func);
    return *this;
}

const std::string& Value::asString() const
{
    return *_u.str;
}

const Buffer& Value::asBuffer() const
{
    return *_u.buf;
}

bool Value::asBool() const
{
    return _u.b;
}

int Value::asInt() const
{
    return _u.i;
}

float Value::asFloat() const
{
    return _u.f;
}

const ValueArray& Value::asArray() const
{
    return *_u.arr;
}

const ValueObject& Value::asObject() const
{
    return *_u.obj;
}

const SocketIOPacket& Value::asSocketIOPacket() const
{
    return *_u.sp;
}

const EngineIOPacket& Value::asEngineIOPacket() const
{
    return *_u.ep;
}

const ValueFunction& Value::asFunction() const
{
    return *_u.func;
}

bool Value::isValid() const
{
    return _type != Type::NONE;
}

bool Value::hasBin() const
{
    bool ret = false;

    if (_type == Type::BINARY) {
        ret = true;
    } else if (_type == Type::ARRAY) {
        const ValueArray& arr = asArray();
        for (const auto& v : arr) {
            if (v.getType() == Type::BINARY)
            {
                ret = true;
                break;
            }
        }
    } else if (_type == Value::Type::OBJECT) {
        const ValueObject& obj = asObject();
        for (const auto& e : obj) {
            if (e.second.getType() == Type::BINARY)
            {
                ret = true;
                break;
            }
        }
    }

    return ret;
}

void Value::reset()
{
    switch (_type)
    {
        case Type::STRING:
            delete _u.str;
            _u.str = nullptr;
            break;
        case Type::BINARY:
            delete _u.buf;
            _u.buf = nullptr;
            break;
        case Type::BOOLEAN:
            _u.b = false;
            break;
        case Type::INTEGER:
            _u.i = 0;
            break;
        case Type::FLOAT:
            _u.f = 0.0f;
            break;
        case Type::ARRAY:
            delete _u.arr;
            _u.arr = nullptr;
            break;
        case Type::OBJECT:
            delete _u.obj;
            _u.obj = nullptr;
            break;
        case Type::FUNCTION:
            delete _u.func;
            _u.func = nullptr;
            break;
        case Type::ENGINEIO_PACKET:
            delete _u.ep;
            _u.ep = nullptr;
            break;
        case Type::SOCKETIO_PACKET:
            delete _u.sp;
            _u.sp = nullptr;
            break;
        default:
            break;
    }
}

std::string Value::toString() const
{
    std::stringstream ss;

    switch (_type)
    {
        case Type::STRING:
            ss << *_u.str;
            break;
        case Type::BINARY:
            ss << "(Buffer: " << _u.buf->length() << ")";
            break;
        case Type::BOOLEAN:
            ss << (_u.b ? "true" : "false");
            break;
        case Type::INTEGER:
            ss << _u.i;
            break;
        case Type::FLOAT:
            ss << _u.f;
            break;
        case Type::ARRAY:
        {
            ss << "[ ";
            for (const auto& e : *_u.arr)
            {
                ss << e.toString() << ", ";
            }
            ss << " ]";
        }
            break;
        case Type::OBJECT:
        {
            ss << "{ ";
            for (const auto& e : *_u.obj)
            {
                ss << e.first << ": " << e.second.toString() << ", ";
            }
            ss << " }";
        }
            break;
        case Type::FUNCTION:
            ss << "Function";
            break;
        case Type::ENGINEIO_PACKET:
            ss << "EngineIOPacket";
            break;
        case Type::SOCKETIO_PACKET:
            ss << "SocketIOPacket";
            break;
        default:
            assert(false);
            break;
    }

    return ss.str();
}

ValueArray Value::concat(const Value& a, const Value& b)
{
    ValueArray ret;

    if (a.getType() == Value::Type::ARRAY)
    {
        ValueArray aArr = a.asArray();
        ret.insert(ret.end(), aArr.begin(), aArr.end());
    }
    else
    {
        ret.push_back(a);
    }

    if (b.getType() == Value::Type::ARRAY)
    {
        ValueArray bArr = b.asArray();
        ret.insert(ret.end(), bArr.begin(), bArr.end());
    }
    else
    {
        ret.push_back(b);
    }

    return ret;
}

//////

SocketIOPacket::SocketIOPacket()
{
    id = -1;
    type = Type::ERROR;
    attachments = -1;
}

SocketIOPacket::SocketIOPacket(const SocketIOPacket& o)
{
    id = o.id;
    nsp = o.nsp;
    type = o.type;
    query = o.query;
    attachments = o.attachments;
    data = o.data;
    options = o.options;
}

SocketIOPacket::SocketIOPacket(SocketIOPacket&& o)
{
    id = o.id;
    nsp = o.nsp;
    type = o.type;
    query = o.query;
    attachments = o.attachments;
    data = o.data;
    options = o.options;

    o.reset();
}

SocketIOPacket::~SocketIOPacket()
{
    reset();
}

SocketIOPacket& SocketIOPacket::operator=(const SocketIOPacket& o)
{
    if (this != &o)
    {
        id = o.id;
        nsp = o.nsp;
        type = o.type;
        query = o.query;
        attachments = o.attachments;
        data = o.data;
        options = o.options;
    }
    return *this;
}

SocketIOPacket& SocketIOPacket::operator=(SocketIOPacket&& o)
{
    if (this != &o)
    {
        id = o.id;
        nsp = o.nsp;
        type = o.type;
        query = o.query;
        attachments = o.attachments;
        data = o.data;
        options = o.options;

        o.reset();
    }
    return *this;
}

bool SocketIOPacket::isValid() const
{
    return false;
}

void SocketIOPacket::reset()
{
    nsp.clear();
    type = Type::ERROR;
    query.clear();
    attachments = -1;
    data.reset();
    options.clear();
}

std::string SocketIOPacket::toString() const
{
    return data.toString();
}

/**
 * Premade error packet.
 */
EngineIOPacket EngineIOPacket::ERROR = {"error", "parser error"};

EngineIOPacket EngineIOPacket::NONE;


///

bool Opts::isValid() const
{
    return false;
}


TimerHandle INVALID_TIMER_HANDLE = -1;

