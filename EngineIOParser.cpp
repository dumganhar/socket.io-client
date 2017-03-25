#include "EngineIOParser.h"

namespace socketio { namespace parser {


uint8_t protocol = 3;

static std::unordered_map<std::string, uint8_t> __packets = {
  { "open": 0 },    // non-ws
  { "close": 1 },   // non-ws
  { "ping": 2 },
  { "pong": 3 },
  { "message": 4 },
  { "upgrade": 5 },
  { "noop": 6 }
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
 * Premade error packet.
 */

struct {
  const char* type;
  const char* data;
} err = {"error", "parser error"};


/**
 * Encodes a packet with binary data in a base64 string
 *
 * @param {Object} packet, has `type` and `data`
 * @return {String} base64 encoded message
 */

std::string encodeBase64Packet(const Packet& packet)
{
  std::string message = "b" + __packets[packet.type];
  message += packet.data.toString("base64");
  return message;
};

/**
 * Encode Buffer data
 */

static Data encodeBuffer(const Packet& packet, bool supportsBinary)
{
  Data& data = packet.data;
  if (!supportsBinary) {
    return encodeBase64Packet(packet, callback);
  }

  var typeBuffer = new Buffer(1);
  typeBuffer[0] = __packets[packet.type];
  return callback(Buffer.concat([typeBuffer, data]));
}

Data encodePacket(const Packet& packet, bool supportsBinary, bool utf8encode)
{
  if (packet.data.isBinary) {
    return encodeBuffer(packet, supportsBinary);
  }

  // encode string
  std::stringstream encoded;
  // Sending data as a utf-8 string
  auto iter = __packets.find(packet.type);
  if (iter == __packets.end())
    return "";

  encoded << iter->second;

  // data fragment is optional
  if (!packet.data.isNull()) {
    if (utf8encode)
    {
      encoded << utf8.encode(String(packet.data));
    }
    else
    {
      encoded << String(packet.data);
    }
  }

  return encoded.str();
}

/**
 * Decodes a packet encoded in a base64 string.
 *
 * @param {String} base64 encoded message
 * @return {Object} with `type` and `data` (if any)
 */

static Packet decodeBase64Packet(const std::string& msg) {
  var type = __packetslist[msg.charAt(0)];
  var data = new Buffer(msg.substr(1), "base64");
  if (binaryType == "arraybuffer") {
    var abv = new Uint8Array(data.length);
    for (var i = 0; i < abv.length; i++){
      abv[i] = data[i];
    }
    data = abv.buffer;
  }
  return { type: type, data: data };
}

Packet decodePacket(const Data& data, bool binaryType, bool utf8decode)
{
  if (!data.isValid()) {
    return err;
  }
  // String data
  if (!data.isBinary()) {
    if (data.charAt(0) == "b") {
      return decodeBase64Packet(data.substr(1), binaryType);
    }

    uint8_t type = data.charAt(0);

    if (utf8decode) {
      data = tryDecode(data);
      if (data == false) {
        return err;
      }
    }

    if (Number(type) != type || !__packetslist[type]) {
      return err;
    }

    if (data.length > 1) {
      return { type: __packetslist[type], data: data.substring(1) };
    } else {
      return { type: __packetslist[type] };
    }
  } else {

    // Binary data
    var type = data[0];
    return { type: __packetslist[type], data: data.slice(1) };
}

function tryDecode(data) {
  try {
    data = utf8.decode(data);
  } catch (e) {
    return false;
  }
  return data;
}

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

static Data encodePayloadAsBinary(const Packet& packets)
{
  if (!packets.length) {
    return callback(new Buffer(0));
  }

  function encodeOne(p, doneCallback) {
    encodePacket(p, true, true, function(packet) {

      if (typeof packet == "string") {
        var encodingLength = "" + packet.length;
        var sizeBuffer = new Buffer(encodingLength.length + 2);
        sizeBuffer[0] = 0; // is a string (not true binary = 0)
        for (var i = 0; i < encodingLength.length; i++) {
          sizeBuffer[i + 1] = parseInt(encodingLength[i], 10);
        }
        sizeBuffer[sizeBuffer.length - 1] = 255;
        return doneCallback(null, Buffer.concat([sizeBuffer, stringToBuffer(packet)]));
      }

      var encodingLength = "" + packet.length;
      var sizeBuffer = new Buffer(encodingLength.length + 2);
      sizeBuffer[0] = 1; // is binary (true binary = 1)
      for (var i = 0; i < encodingLength.length; i++) {
        sizeBuffer[i + 1] = parseInt(encodingLength[i], 10);
      }
      sizeBuffer[sizeBuffer.length - 1] = 255;
      doneCallback(null, Buffer.concat([sizeBuffer, packet]));
    });
  }

  map(packets, encodeOne, function(err, results) {
    return callback(Buffer.concat(results));
  });
}

Data encodePayload(const Packet& packet, bool supportsBinary)
{
  if (supportsBinary) {
    return exports.encodePayloadAsBinary(packets, callback);
  }

  if (!packets.length) {
    return callback("0:");
  }

  function setLengthHeader(message) {
    return message.length + ":" + message;
  }

  function encodeOne(packet, doneCallback) {
    exports.encodePacket(packet, supportsBinary, true, function(message) {
      doneCallback(null, setLengthHeader(message));
    });
  }

  map(packets, encodeOne, function(err, results) {
    return callback(results.join(""));
  });
}

/**
 * Async array map using after
 */

function map(ary, each, done) {
  var result = new Array(ary.length);
  var next = after(ary.length, done);

  var eachWithIndex = function(i, el, cb) {
    each(el, function(error, msg) {
      result[i] = msg;
      cb(error, result);
    });
  };

  for (var i = 0; i < ary.length; i++) {
    eachWithIndex(i, ary[i], next);
  }
}

/*
 * Decodes data when a payload is maybe expected. Strings are decoded by
 * interpreting each byte as a key code for entries marked to start with 0. See
 * description of encodePayloadAsBinary

 * @param {Buffer} data, callback method
 * @api public
 */

static Packet decodePayloadAsBinary(const Data& data, bool binaryType)
{
  var bufferTail = data;
  var buffers = [];

  while (bufferTail.length > 0) {
    var strLen = "";
    var isString = bufferTail[0] == 0;
    var numberTooLong = false;
    for (var i = 1; ; i++) {
      if (bufferTail[i] == 255)  break;
      // 310 = char length of Number.MAX_VALUE
      if (strLen.length > 310) {
        numberTooLong = true;
        break;
      }
      strLen += "" + bufferTail[i];
    }
    if(numberTooLong) return callback(err, 0, 1);
    bufferTail = bufferTail.slice(strLen.length + 1);

    var msgLength = parseInt(strLen, 10);

    var msg = bufferTail.slice(1, msgLength + 1);
    if (isString) msg = bufferToString(msg);
    buffers.push(msg);
    bufferTail = bufferTail.slice(msgLength + 1);
  }

  var total = buffers.length;
  buffers.forEach(function(buffer, i) {
    callback(decodePacket(buffer, binaryType, true), i, total);
  });
}

/*
 * Decodes data when a payload is maybe expected. Possible binary contents are
 * decoded from their base64 representation
 *
 * @param {String} data, callback method
 * @api public
 */

Packet decodePayload(const Data& data, bool binaryType)
{
  if ("string" != typeof data) {
    return exports.decodePayloadAsBinary(data, binaryType, callback);
  }

  if (typeof binaryType == "function") {
    callback = binaryType;
    binaryType = null;
  }

  var packet;
  if (data == "") {
    // parser error - ignoring payload
    return callback(err, 0, 1);
  }

  var length = ""
    , n, msg;

  for (var i = 0, l = data.length; i < l; i++) {
    var chr = data.charAt(i);

    if (":" != chr) {
      length += chr;
    } else {
      if ("" == length || (length != (n = Number(length)))) {
        // parser error - ignoring payload
        return callback(err, 0, 1);
      }

      msg = data.substr(i + 1, n);

      if (length != msg.length) {
        // parser error - ignoring payload
        return callback(err, 0, 1);
      }

      if (msg.length) {
        packet = exports.decodePacket(msg, binaryType, true);

        if (err.type == packet.type && err.data == packet.data) {
          // parser error in individual packet - ignoring payload
          return callback(err, 0, 1);
        }

        var ret = callback(packet, i + n, l);
        if (false == ret) return;
      }

      // advance cursor
      i += n;
      length = "";
    }
  }

  if (length != "") {
    // parser error - ignoring payload
    return callback(err, 0, 1);
  }

}

/**
 *
 * Converts a buffer to a utf8.js encoded string
 *
 * @api private
 */

function bufferToString(buffer) {
  var str = "";
  for (var i = 0; i < buffer.length; i++) {
    str += String.fromCharCode(buffer[i]);
  }
  return str;
}

/**
 *
 * Converts a utf8.js encoded string to a buffer
 *
 * @api private
 */

function stringToBuffer(string) {
  var buf = new Buffer(string.length);
  for (var i = 0; i < string.length; i++) {
    buf.writeUInt8(string.charCodeAt(i), i);
  }
  return buf;
}

/**
 *
 * Converts an ArrayBuffer to a Buffer
 *
 * @api private
 */

function arrayBufferToBuffer(data) {
  // data is either an ArrayBuffer or ArrayBufferView.
  var array = new Uint8Array(data.buffer || data);
  var length = data.byteLength || data.length;
  var offset = data.byteOffset || 0;
  var buffer = new Buffer(length);

  for (var i = 0; i < length; i++) {
    buffer[i] = array[offset + i];
  }
  return buffer;
}

}} //namespace socketio { namespace parser {

