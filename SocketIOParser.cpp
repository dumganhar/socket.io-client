#include "SocketIOParser.h"

namespace socketio { namespace parser {

/**
 * Protocol version.
 *
 * @api public
 */

uint8_t protocol = 4;

/**
 * Packet types.
 *
 * @api public
 */

static std::vector<std::string> __types = {
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

Data Encoder::encode(const SocketIOPacket& obj)
{
  debug('encoding packet %j', obj);

  if (SocketIOPacket::Type::BINARY_EVENT == obj.type || SocketIOPacket::Type::BINARY_ACK == obj.type) {
    return encodeAsBinary(obj);
  }
  else {
    return encodeAsString(obj);
  }
}

std::string Encoder::encodeAsString(const SocketIOPacket& obj)
{
  std::string str = "";
  bool nsp = false;

  // first is type
  str += obj.getType();

  // attachments if we have them
  if (SocketIOPacket::Type::BINARY_EVENT == obj.type || SocketIOPacket::Type::BINARY_ACK == obj.type) {
    str += obj.getAttachments();
    str += '-';
  }

  // if we have a namespace other than `/`
  // we append it followed by a comma `,`
  if (!obj.getNsp().empty() && "/" != obj.getNsp()) {
    nsp = true;
    str += obj.getNsp();
  }

  // immediately followed by the id
  if (!obj.getId().empty()) {
    if (nsp) {
      str += ",";
      nsp = false;
    }
    str += obj.getId();
  }

  // json data
  if (obj.getData().isValid()) {
    if (nsp) str += ",";
    str += json.stringify(obj.data);
  }

  debug('encoded %j as %s', obj, str);
  return str;
}

Data Encoder::encodeAsBinary(const SocketIOPacket& obj)
{
  function writeEncoding(bloblessData) {
    var deconstruction = binary.deconstructPacket(bloblessData);
    std::string pack = encodeAsString(deconstruction.packet);
    var buffers = deconstruction.buffers;

    buffers.unshift(pack); // add packet info to beginning of data list
    callback(buffers); // write all the buffers
  }

  binary.removeBlobs(obj, writeEncoding);
}


//

/**
 * A manager of a binary event's 'buffer sequence'. Should
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

  SocketIOPacket takeBinaryData(const Data& binData);

  /**
   * Cleans up binary packet reconstruction variables.
   *
   * @api private
   */

  void finishedReconstruction();

private:
  SocketIOPacket _reconPack;
  std::vector<Data> _buffers;
};

BinaryReconstructor::BinaryReconstructor(const SocketIOPacket& packet)
: _reconPack(packet)
{
}

SocketIOPacket BinaryReconstructor::takeBinaryData(const Data& binData)
{
  SocketIOPacket packet;
  _buffers.push_back(binData);
  if (_buffers.size() == _reconPack.getAttachments()) { // done with buffer list
    packet = binary.reconstructPacket(_reconPack, _buffers);
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

}

void Decoder::add(const Data& obj)
{
  var packet;
  if ('string' == typeof obj) {
    packet = decodeString(obj);
    if (exports.BINARY_EVENT == packet.type || exports.BINARY_ACK == packet.type) { // binary packet's json
      this.reconstructor = new BinaryReconstructor(packet);

      // no attachments, labeled binary but no binary data to follow
      if (this.reconstructor.reconPack.attachments == 0) {
        this.emit('decoded', packet);
      }
    } else { // non-binary full packet
      this.emit('decoded', packet);
    }
  }
  else if (isBuf(obj) || obj.base64) { // raw binary data
    if (!this.reconstructor) {
      throw new Error('got binary data when not reconstructing a packet');
    } else {
      packet = this.reconstructor.takeBinaryData(obj);
      if (packet) { // received final buffer
        this.reconstructor = null;
        this.emit('decoded', packet);
      }
    }
  }
  else {
    throw new Error('Unknown type: ' + obj);
  }
};

SocketIOPacket Decoder::decodeString(const std::string& str)
{
  SocketIOPacket p;
  size_t i = 0;

  // look up type
  p.type = Number(str.charAt(0));

  if (p.type < 0 || p.type >= __types.size())
    return false;

  // look up attachments if type binary
  if (SocketIOPacket::Type::BINARY_EVENT == p.type || SocketIOPacket::Type::BINARY_ACK == p.type) {
    var buf = '';
    while (str.charAt(++i) != '-') {
      buf += str.charAt(i);
      if (i == str.length) break;
    }
    if (buf != Number(buf) || str.charAt(i) != '-') {
      throw new Error('Illegal attachments');
    }
    p.attachments = Number(buf);
  }

  // look up namespace (if any)
  if ('/' == str.charAt(i + 1)) {
    p.nsp = '';
    while (++i) {
      var c = str.charAt(i);
      if (',' == c) break;
      p.nsp += c;
      if (i == str.length) break;
    }
  } else {
    p.nsp = '/';
  }

  // look up id
  var next = str.charAt(i + 1);
  if ('' != next && Number(next) == next) {
    p.id = '';
    while (++i) {
      var c = str.charAt(i);
      if (null == c || Number(c) != c) {
        --i;
        break;
      }
      p.id += str.charAt(i);
      if (i == str.length) break;
    }
    p.id = Number(p.id);
  }

  // look up json data
  if (str.charAt(++i)) {
    p = tryParse(p, str.substr(i));
  }

  debug('decoded %s as %j', str, p);
  return p;
}

function tryParse(p, str) {
  try {
    p.data = json.parse(str);
  } catch(e){
    return error();
  }
  return p; 
};

void Decoder::destroy()
{
  if (this.reconstructor) {
    this.reconstructor.finishedReconstruction();
  }
};



function error(data){
  return {
    type: exports.ERROR,
    data: 'parser error'
  };
}

}} // namespace socketio { namespace parser {