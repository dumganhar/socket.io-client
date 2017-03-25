#include "SocketIOBinary.h"

DeconstructedPacket deconstructPacket(const SocketIOPacket& packet)
{
  ValueArray buffers;
  const Value& packetData = packet.getData();

  auto _deconstructPacket = [](const Value& data) -> Value {
    if (!data) return data;

    if (data.getType() == Value::Type::BINARY) {
      std::stringstream placeholder;
      placeholder << "{ _placeholder: true, num:" << (int)buffers.size() << " }";
      buffers.push_back(data);
      return placeholder.str();
    } else if (data.getType() == Value::Type::ARRAY) {
      ValueArray newData;
      const ValueArray& originalArr = data.asArray();
      newData.resize(originalArr.size());
      for (size_t i = 0; i < originalArr.size(); i++) {
        newData[i] = _deconstructPacket(originalArr[i]);
      }
      return newData;
    } else if (data.getType() == Value::Type::OBJECT) {
      ValueObject newData;
      const ValueObject& originalObj = data.asObject();
      for (const auto& e : originalObj) {
        newData[e.first] = _deconstructPacket(e.second);
      }
      return newData;
    }
    return data;
  }
  
  SocketIOPacket pack = packet;
  pack.data = _deconstructPacket(packetData);
  pack.attachments = buffers.size(); // number of binary 'attachments'

  DeconstructedPacket ret;
  ret.packet = pack;
  ret.buffers = buffers;
  return ret;
}

SocketIOPacket reconstructPacket(const SocketIOPacket& packet, const ValueArray& buffers)
{
  int curPlaceHolder = 0;

  auto _reconstructPacket = [](const Value& data) -> Value {
    if (data && data._placeholder) {
      Value& buf = buffers[data.num]; // appropriate buffer (should be natural order anyway)
      return buf;
    } else if (data.getType() == Value::Type::ARRAY) {
      ValueArray arr;
      const ValueArray& originalArr = data.asArray();
      for (size_t i = 0; i < originalArr.size(); i++) {
        arr[i] = _reconstructPacket(data[i]);
      }
      return arr;
    } else if (data.getType() == Value::Type::OBJECT) {
      ValueObject obj;
      const ValueObject& originalObj = data.asObject();
      for (const auto& e : originalObj) {
        obj[e.first] = _reconstructPacket(e.second);
      }
      return obj;
    }
    return data;
  };

  SocketIOPacket p;
  p.setData(_reconstructPacket(packet.getData()));
  p.setAttachments(-1); // no longer useful
  return p;
};

/**
 * Asynchronously removes Blobs or Files from data via
 * FileReader's readAsArrayBuffer method. Used before encoding
 * data as msgpack. Calls callback with the blobless data.
 *
 * @param {Object} data
 * @param {Function} callback
 * @api private
 */

exports.removeBlobs = function(data, callback) {
  function _removeBlobs(obj, curKey, containingObject) {
    if (!obj) return obj;

    // convert any blob
    if ((global.Blob && obj instanceof Blob) ||
        (global.File && obj instanceof File)) {
      pendingBlobs++;

      // async filereader
      var fileReader = new FileReader();
      fileReader.onload = function() { // this.result == arraybuffer
        if (containingObject) {
          containingObject[curKey] = this.result;
        }
        else {
          bloblessData = this.result;
        }

        // if nothing pending its callback time
        if(! --pendingBlobs) {
          callback(bloblessData);
        }
      };

      fileReader.readAsArrayBuffer(obj); // blob -> arraybuffer
    } else if (isArray(obj)) { // handle array
      for (var i = 0; i < obj.length; i++) {
        _removeBlobs(obj[i], i, obj);
      }
    } else if (obj && 'object' == typeof obj && !isBuf(obj)) { // and object
      for (var key in obj) {
        _removeBlobs(obj[key], key, obj);
      }
    }
  }

  var pendingBlobs = 0;
  var bloblessData = data;
  _removeBlobs(bloblessData);
  if (!pendingBlobs) {
    callback(bloblessData);
  }
};
