#include "EngineIOParser.h"
#include "IOUtils.h"
#include <assert.h>

namespace engineio { namespace parser {


uint8_t getProtocolVersion()
{
    return 3;
}

static std::unordered_map<std::string, uint8_t> __packets = {
  { "open", 0 },    // non-ws
  { "close", 1 },   // non-ws
  { "ping", 2 },
  { "pong", 3 },
  { "message", 4 },
  { "upgrade", 5 },
  { "noop", 6 }
};
/**
 * Packet types.
 */

static const std::vector<std::string> __packetslist = {
  "open",
  "close"
  "ping",
  "pong",
  "message"
  "upgrade",
  "noop"
};

/**
 * Encodes a packet with binary data in a base64 string
 *
 * @param {Object} packet, has `type` and `data`
 * @return {String} base64 encoded message
 */

std::string encodeBase64Packet(const EngineIOPacket& packet)
{
    std::string message = std::string("b") + toString(__packets[packet.type]);
    assert(packet.data.getType() == Value::Type::BINARY);

    message += packet.data.asBuffer().toBase64String();
    return message;
}

/**
 * Encode Buffer data
 */

static Value encodeBuffer(const EngineIOPacket& packet, bool supportsBinary)
{
    assert(packet.data.getType() == Value::Type::BINARY);
  const Value& data = packet.data;
  if (!supportsBinary) {
    return encodeBase64Packet(packet);
  }

   const Buffer& d = data.asBuffer();

    Buffer buf(nullptr, d.length() + 1);
    buf.setData(0, &__packets[packet.type], 1);
    buf.setData(1, d.data(), d.length());
    return buf;
}

Value encodePacket(const EngineIOPacket& packet, bool supportsBinary, bool utf8encode)
{
    if (packet.data.getType() == Value::Type::BINARY) {
        return encodeBuffer(packet, supportsBinary);
    }

  // encode string
  std::stringstream encoded;
  // Sending data as a utf-8 string
  auto iter = __packets.find(packet.type);
  if (iter == __packets.end())
    return "";

  encoded << iter->second;

    assert(packet.data.getType() == Value::Type::STRING);

  // data fragment is optional
  if (packet.data.isValid()) {
//cjh    if (utf8encode)
//    {
//      encoded << utf8Encode(String(packet.data));
//    }
//    else
//    {
//      encoded << String(packet.data);
//    }
  }

  return encoded.str();
}

/**
 * Decodes a packet encoded in a base64 string.
 *
 * @param {String} base64 encoded message
 * @return {Object} with `type` and `data` (if any)
 */

static EngineIOPacket decodeBase64Packet(const std::string& msg) {
    std::string type = __packetslist[msg[0]];
    std::string base64 = msg.substr(1);
    Buffer data = base64Decode(base64);

    EngineIOPacket packet;
    packet.type = type;
    packet.data = data;
    return packet;
}

EngineIOPacket decodePacket(const Value& data, bool utf8decode)
{
  if (!data.isValid()) {
      return EngineIOPacket::ERROR;
  }

    EngineIOPacket ret;
  // String data
    if (data.getType() == Value::Type::STRING) {
        const std::string& str = data.asString();
        if (str[0] == 'b') {
          return decodeBase64Packet(str.substr(1));
        }

        uint8_t type = str[0];

        std::string decodedStr;
        if (utf8decode) {
          decodedStr = utf8Decode(str);
            if (decodedStr.empty()){
            return EngineIOPacket::ERROR;
          }
        }

        if (type >= __packetslist.size()) {
          return EngineIOPacket::ERROR;
        }

        ret.type = __packetslist[type];
        if (str.length() > 1) {
            ret.data = str.substr(1);
        }
    } else if (data.getType() == Value::Type::BINARY) {

        // Binary data
        const Buffer& buf = data.asBuffer();
        uint8_t type = buf[0];
        ret.type = __packetslist[type];
        ret.data = std::move(Buffer(buf.data() + 1, buf.length() - 1));
    }

    return ret;
}

//function tryDecode(data) {
//  try {
//    data = utf8.decode(data);
//  } catch (e) {
//    return false;
//  }
//  return data;
//}

/**
 * Encodes multiple messages (payload) as binary.
 *
 * <1 = binary, 0 = string><number from 0-9><number from 0-9>[...]<number
 * 255><data>
 *
 * Example:
 * 1 3 255 1 2 3, if the binary contents are interpreted as 8 bit integers
 *
 * @param {Array} packets
 * @return {Buffer} encoded payload
 * @api private
 */

static Value encodePayloadAsBinary(const EngineIOPacket& packets)
{
//  if (!packets.isValid()) {
//      return Value::NONE;
//  }
//
//  auto encodeOne = [](const EngineIOPacket& p, doneCallback) {
//      EngineIOPacket encoded = encodePacket(p, true, true);
//
//      if (typeof packet == "string") {
//        var encodingLength = "" + packet.length;
//        var sizeBuffer = new Buffer(encodingLength.length + 2);
//        sizeBuffer[0] = 0; // is a string (not true binary = 0)
//        for (var i = 0; i < encodingLength.length; i++) {
//          sizeBuffer[i + 1] = parseInt(encodingLength[i], 10);
//        }
//        sizeBuffer[sizeBuffer.length - 1] = 255;
//        return doneCallback(null, Buffer.concat([sizeBuffer, stringToBuffer(packet)]));
//      }
//
//      var encodingLength = "" + packet.length;
//      var sizeBuffer = new Buffer(encodingLength.length + 2);
//      sizeBuffer[0] = 1; // is binary (true binary = 1)
//      for (var i = 0; i < encodingLength.length; i++) {
//        sizeBuffer[i + 1] = parseInt(encodingLength[i], 10);
//      }
//      sizeBuffer[sizeBuffer.length - 1] = 255;
//      doneCallback(null, Buffer.concat([sizeBuffer, packet]));
//    });
//  };
//
//  map(packets, encodeOne, function(err, results) {
//    return callback(Buffer.concat(results));
//  });

    return Value::NONE;
}

Value encodePayload(const std::vector<EngineIOPacket>& packet, bool supportsBinary)
{
//  if (supportsBinary) {
//    return exports.encodePayloadAsBinary(packets, callback);
//  }
//
//  if (!packets.length) {
//    return callback("0:");
//  }
//
//  function setLengthHeader(message) {
//    return message.length + ":" + message;
//  }
//
//  function encodeOne(packet, doneCallback) {
//    exports.encodePacket(packet, supportsBinary, true, function(message) {
//      doneCallback(null, setLengthHeader(message));
//    });
//  }
//
//  map(packets, encodeOne, function(err, results) {
//    return callback(results.join(""));
//  });

    return Value::NONE;
}

/**
 * Async array map using after
 */

//function map(ary, each, done) {
//  var result = new Array(ary.length);
//  var next = after(ary.length, done);
//
//  var eachWithIndex = function(i, el, cb) {
//    each(el, function(error, msg) {
//      result[i] = msg;
//      cb(error, result);
//    });
//  };
//
//  for (var i = 0; i < ary.length; i++) {
//    eachWithIndex(i, ary[i], next);
//  }
//}

/*
 * Decodes data when a payload is maybe expected. Strings are decoded by
 * interpreting each byte as a key code for entries marked to start with 0. See
 * description of encodePayloadAsBinary

 * @param {Buffer} data, callback method
 * @api public
 */

static EngineIOPacket decodePayloadAsBinary(const Value& data, bool binaryType)
{
//  var bufferTail = data;
//  var buffers = [];
//
//  while (bufferTail.length > 0) {
//    var strLen = "";
//    var isString = bufferTail[0] == 0;
//    var numberTooLong = false;
//    for (var i = 1; ; i++) {
//      if (bufferTail[i] == 255)  break;
//      // 310 = char length of Number.MAX_VALUE
//      if (strLen.length > 310) {
//        numberTooLong = true;
//        break;
//      }
//      strLen += "" + bufferTail[i];
//    }
//    if(numberTooLong) return callback(err, 0, 1);
//    bufferTail = bufferTail.slice(strLen.length + 1);
//
//    var msgLength = parseInt(strLen, 10);
//
//    var msg = bufferTail.slice(1, msgLength + 1);
//    if (isString) msg = bufferToString(msg);
//    buffers.push(msg);
//    bufferTail = bufferTail.slice(msgLength + 1);
//  }
//
//  var total = buffers.length;
//  buffers.forEach(function(buffer, i) {
//    callback(decodePacket(buffer, binaryType, true), i, total);
//  });
    return EngineIOPacket::NONE;
}

/*
 * Decodes data when a payload is maybe expected. Possible binary contents are
 * decoded from their base64 representation
 *
 * @param {String} data, callback method
 * @api public
 */

EngineIOPacket decodePayload(const Value& data)
{
//  if ("string" != typeof data) {
//    return exports.decodePayloadAsBinary(data, binaryType, callback);
//  }
//
//  if (typeof binaryType == "function") {
//    callback = binaryType;
//    binaryType = null;
//  }
//
//  var packet;
//  if (data == "") {
//    // parser error - ignoring payload
//    return callback(err, 0, 1);
//  }
//
//  var length = ""
//    , n, msg;
//
//  for (var i = 0, l = data.length; i < l; i++) {
//    var chr = data.charAt(i);
//
//    if (":" != chr) {
//      length += chr;
//    } else {
//      if ("" == length || (length != (n = Number(length)))) {
//        // parser error - ignoring payload
//        return callback(err, 0, 1);
//      }
//
//      msg = data.substr(i + 1, n);
//
//      if (length != msg.length) {
//        // parser error - ignoring payload
//        return callback(err, 0, 1);
//      }
//
//      if (msg.length) {
//        packet = exports.decodePacket(msg, binaryType, true);
//
//        if (err.type == packet.type && err.data == packet.data) {
//          // parser error in individual packet - ignoring payload
//          return callback(err, 0, 1);
//        }
//
//        var ret = callback(packet, i + n, l);
//        if (false == ret) return;
//      }
//
//      // advance cursor
//      i += n;
//      length = "";
//    }
//  }
//
//  if (length != "") {
//    // parser error - ignoring payload
//    return callback(err, 0, 1);
//  }
    return EngineIOPacket::NONE;
}

/**
 *
 * Converts a buffer to a utf8.js encoded string
 *
 * @api private
 */

//function bufferToString(buffer) {
//  var str = "";
//  for (var i = 0; i < buffer.length; i++) {
//    str += String.fromCharCode(buffer[i]);
//  }
//  return str;
//}

/**
 *
 * Converts a utf8.js encoded string to a buffer
 *
 * @api private
 */

//function stringToBuffer(string) {
//  var buf = new Buffer(string.length);
//  for (var i = 0; i < string.length; i++) {
//    buf.writeUInt8(string.charCodeAt(i), i);
//  }
//  return buf;
//}

/**
 *
 * Converts an ArrayBuffer to a Buffer
 *
 * @api private
 */

//function arrayBufferToBuffer(data) {
//  // data is either an ArrayBuffer or ArrayBufferView.
//  var array = new Uint8Array(data.buffer || data);
//  var length = data.byteLength || data.length;
//  var offset = data.byteOffset || 0;
//  var buffer = new Buffer(length);
//
//  for (var i = 0; i < length; i++) {
//    buffer[i] = array[offset + i];
//  }
//  return buffer;
//}

}} //namespace socketio { namespace parser {

