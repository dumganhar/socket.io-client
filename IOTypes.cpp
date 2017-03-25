#include "IOTypes.h"

#include <sstream>
#include <stdlib.h>
#include <string.h>

Buffer::Buffer()
: _data(nullptr)
, _len(0)
, _isBinary(false)
{

}

Buffer::Buffer(uint8_t* data, size_t len)
: _len(len)
, _isBinary(true)
{
    _data = (uint8_t*) malloc(_len);
    memcpy(_data, data, len);
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

uint8_t Buffer::operator[](int index)
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
///

static const std::string EMPTY_STRING;

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
        case Type::PACKET:
            _u.packet = new SocketIOPacket(*o._u.packet);
            break;
        default:
            break;
    }
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

Value::Value(const SocketIOPacket& packet)
{
    _type = Type::PACKET;
    _u.packet = new SocketIOPacket(packet);
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
            case Type::PACKET:
                _u.packet = new SocketIOPacket(*o._u.packet);
                break;
            default:
                break;
        }
    }
    return *this;
}

Value& Value::operator=(const std::string& str)
{
    _type = Type::STRING;
    _u.str = new std::string(str);
    return *this;
}

Value& Value::operator=(const Buffer& buf)
{
    _type = Type::BINARY;
    _u.buf = new Buffer(buf);
    return *this;
}

Value& Value::operator=(bool v)
{
    _type = Type::BOOLEAN;
    _u.b = v;
    return *this;
}

Value& Value::operator=(int intVal)
{
    _type = Type::INTEGER;
    _u.i = intVal;
    return *this;
}

Value& Value::operator=(float floatVal)
{
    _type = Type::FLOAT;
    _u.f = floatVal;
    return *this;
}

Value& Value::operator=(const ValueArray& arrVal)
{
    _type = Type::ARRAY;
    _u.arr = new ValueArray(arrVal);
    return *this;
}

Value& Value::operator=(const ValueObject& objVal)
{
    _type = Type::OBJECT;
    _u.obj = new ValueObject(objVal);
    return *this;
}

Value& Value::operator=(const SocketIOPacket& packet)
{
    _type = Type::PACKET;
    _u.packet = new SocketIOPacket(packet);
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

const SocketIOPacket& Value::asPacket() const
{
    return *_u.packet;
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
        case Type::PACKET:
            delete _u.packet;
            _u.packet = nullptr;
            break;
        default:
            break;
    }
}

std::string Value::toString() const
{
    return "";
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
    eventName = o.eventName;
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
    eventName = o.eventName;
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
        eventName = o.eventName;
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
        eventName = o.eventName;
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
    eventName.clear();
    nsp.clear();
    type = Type::ERROR;
    query.clear();
    attachments = -1;
    data.reset();
    options.clear();
}

std::string SocketIOPacket::toString() const
{
    return "";
}