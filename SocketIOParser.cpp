#include "SocketIOParser.h"

#include "SocketIOBinary.h"
#include "IOUtils.h"

namespace socketio { namespace parser {

/**
 * Protocol version.
 *
 * @api public
 */

uint8_t getProtocolVersion()
{
  return 4;
}


/**
 * Packet types.
 *
 * @api public
 */

static const std::vector<std::string> __types = {
  "CONNECT",
  "DISCONNECT",
  "EVENT",
  "ACK",
  "ERROR",
  "BINARY_EVENT",
  "BINARY_ACK"
};

Encoder::Encoder()
{

}

Encoder::~Encoder()
{

}

ValueArray Encoder::encode(const SocketIOPacket& obj)
{
  debug("encoding packet %s\n", obj.toString().c_str());

  if (SocketIOPacket::Type::BINARY_EVENT == obj.type || SocketIOPacket::Type::BINARY_ACK == obj.type) {
    return encodeAsBinary(obj);
  }
  else {
      ValueArray arr;
      arr.push_back(encodeAsString(obj));
    return arr;
  }
}

std::string Encoder::encodeAsString(const SocketIOPacket& obj)
{
  std::string str = "";
  bool nsp = false;

    // first is type
    str += toString((int)obj.type);

  // attachments if we have them
  if (SocketIOPacket::Type::BINARY_EVENT == obj.type || SocketIOPacket::Type::BINARY_ACK == obj.type) {
    str += toString(obj.attachments);
    str += "-";
  }

  // if we have a namespace other than `/`
  // we append it followed by a comma `,`
  if (!obj.nsp.empty() && "/" != obj.nsp) {
    nsp = true;
    str += obj.nsp;
  }

  // immediately followed by the id
  if (obj.id != -1) {
    if (nsp) {
      str += ",";
      nsp = false;
    }
    str += obj.id;
  }

  // json data
  if (obj.data.isValid()) {
    if (nsp) str += ",";
//cjh    str += json.stringify(obj.data);
  }

  debug("encoded %s as %s", obj.toString().c_str(), str.c_str());
  return str;
}

ValueArray Encoder::encodeAsBinary(const SocketIOPacket& obj)
{
    binary::DeconstructedPacket deconstruction = binary::deconstructPacket(obj);
    std::string pack = encodeAsString(deconstruction.packet);
    ValueArray& buffers = deconstruction.buffers;

    buffers.insert(buffers.begin(), pack); // add packet info to beginning of data list
    return buffers;
}


//

/**
 * A manager of a binary event"s 'buffer sequence". Should
 * be constructed whenever a packet of type BINARY_EVENT is
 * decoded.
 *
 * @param {Object} packet
 * @return {BinaryReconstructor} initialized reconstructor
 * @api private
 */

class BinaryReconstructor
{
public:
  BinaryReconstructor(const SocketIOPacket& packet);

  /**
   * Method to be called when binary data received from connection
   * after a BINARY_EVENT packet.
   *
   * @param {Buffer | ArrayBuffer} binData - the raw binary data received
   * @return {null | Object} returns null if more binary data is expected or
   *   a reconstructed packet object if all buffers have been received.
   * @api private
   */

  SocketIOPacket takeBinaryData(const Value& binData);

  /**
   * Cleans up binary packet reconstruction variables.
   *
   * @api private
   */

  void finishedReconstruction();

//private:
  SocketIOPacket _reconPack;
  ValueArray _buffers;
};

BinaryReconstructor::BinaryReconstructor(const SocketIOPacket& packet)
: _reconPack(packet)
{
}

SocketIOPacket BinaryReconstructor::takeBinaryData(const Value& binData)
{
  SocketIOPacket packet;
  _buffers.push_back(binData);
  if (_buffers.size() == _reconPack.attachments) { // done with buffer list
    packet = binary::reconstructPacket(_reconPack, _buffers);
    finishedReconstruction();
  }
  return packet;
}

void BinaryReconstructor::finishedReconstruction()
{
  _reconPack.reset();
  _buffers.clear();
}

//

Decoder::Decoder()
: _reconstructor(nullptr)
{

}

Decoder::~Decoder()
{
  delete _reconstructor;
}

bool Decoder::add(const Value& obj)
{
  SocketIOPacket packet;
  if (obj.getType() == Value::Type::STRING) {
    packet = decodeString(obj.asString());
    if (SocketIOPacket::Type::BINARY_EVENT == packet.type || SocketIOPacket::Type::BINARY_ACK == packet.type) { // binary packet's json
      _reconstructor = new BinaryReconstructor(packet);

      // no attachments, labeled binary but no binary data to follow
      if (_reconstructor->_reconPack.attachments == 0) {
        emit("decoded", packet);
      }
    } else { // non-binary full packet
      emit("decoded", packet);
    }
  }
  else if (obj.getType() == Value::Type::BINARY) {// cjh || obj.base64) { // raw binary data
    if (!_reconstructor) {
//cjh      Error("got binary data when not reconstructing a packet");
      return false;
    } else {
      packet = _reconstructor->takeBinaryData(obj);
      if (packet.isValid()) { // received final buffer
        delete _reconstructor;
        _reconstructor = nullptr;
        emit("decoded", packet);
      }
    }
  }
  else {
//cjh    Error("Unknown type: " + obj);
    return false;
  }

  return true;
}

SocketIOPacket Decoder::decodeString(const std::string& str)
{
  SocketIOPacket p;
//  size_t i = 0;
//
//  // look up type
//  p.type = Number(str.charAt(0));
//
//  if (p.type < 0 || p.type >= __types.size())
//    return false;
//
//  // look up attachments if type binary
//  if (SocketIOPacket::Type::BINARY_EVENT == p.type || SocketIOPacket::Type::BINARY_ACK == p.type) {
//    var buf = "";
//    while (str.charAt(++i) != "-") {
//      buf += str.charAt(i);
//      if (i == str.length) break;
//    }
//    if (buf != Number(buf) || str.charAt(i) != "-") {
//      throw new Error("Illegal attachments");
//    }
//    p.attachments = Number(buf);
//  }
//
//  // look up namespace (if any)
//  if ("/" == str.charAt(i + 1)) {
//    p.nsp = "";
//    while (++i) {
//      var c = str.charAt(i);
//      if ("," == c) break;
//      p.nsp += c;
//      if (i == str.length) break;
//    }
//  } else {
//    p.nsp = "/";
//  }
//
//  // look up id
//  var next = str.charAt(i + 1);
//  if ("" != next && Number(next) == next) {
//    p.id = "";
//    while (++i) {
//      var c = str.charAt(i);
//      if (null == c || Number(c) != c) {
//        --i;
//        break;
//      }
//      p.id += str.charAt(i);
//      if (i == str.length) break;
//    }
//    p.id = Number(p.id);
//  }
//
//  // look up json data
//  if (str.charAt(++i)) {
//    p = tryParse(p, str.substr(i));
//  }
//
//  debug("decoded %s as %j", str, p);
  return p;
}

//function tryParse(p, str) {
//  try {
//    p.data = json.parse(str);
//  } catch(e){
//    return error();
//  }
//  return p; 
//};

void Decoder::destroy()
{
  if (_reconstructor) {
    _reconstructor->finishedReconstruction();
  }
};



//function error(data){
//  return {
//    type: exports.ERROR,
//    data: "parser error"
//  };
//}

}} // namespace socketio { namespace parser {
