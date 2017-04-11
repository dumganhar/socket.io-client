#include "EngineIOPolling.h"
#include "EngineIOParser.h"

EngineIOPolling::EngineIOPolling(const ValueObject& opts)
: EngineIOTransport(opts)
{

}

const std::string& EngineIOPolling::getName() const
{
    static std::string name = "polling";
    return name;
}

bool EngineIOPolling::doOpen()
{
    poll();
    return true;
}

void EngineIOPolling::pause(const std::function<void()>& fn)
{
    _readyState = ReadyState::PAUSING;

  auto pauseFunc = [this]() {
      debug("paused");
      _readyState = ReadyState::PAUSED;
      onPause();
  };

  if (_polling || !_writable) {
      std::shared_ptr<int> total = std::make_shared<int>(0);

    if (_polling) {
      debug("we are currently polling - waiting to pause");
      (*total)++;
      once("pollComplete", [=](const Value&) {
        debug("pre-pause polling complete");
        if (--(*total) == 0)
            pauseFunc();
      });
    }

    if (!_writable) {
      debug("we are currently writing - waiting to pause");
      (*total)++;
      once("drain", [=](const Value&) {
          debug("pre-pause writing complete");
          if (--(*total) == 0)
              pauseFunc();
      });
    }
  } else {
    pauseFunc();
  }
}

void EngineIOPolling::poll()
{
  debug("polling");
  _polling = true;
  doPoll();
  emit("poll");
}

void EngineIOPolling::onData(const Value& data)
{
  debug("polling got data %s", data.toString().c_str());

    // decode payload
    EngineIOPacket packet = engineio::parser::decodePayload(data);
    // if its the first message we consider the transport open
    if (ReadyState::OPENING == _readyState) {
        onOpen();
    }

    // if its a close packet, we close the ongoing requests
    if ("close" == packet.type) {
        onClose();
        return;
    }

    // otherwise bypass onData and handle the message
    onPacket(packet);

    // if an event did not trigger closing
    if (ReadyState::CLOSED != _readyState) {
        // if we got data we're not polling
        _polling = false;
        emit("pollComplete");

        if (ReadyState::OPENED == _readyState) {
            poll();
        } else {
            debug("ignoring poll - transport state %d", (int)_readyState);
        }
    }
}

void EngineIOPolling::doClose()
{
  auto close = [this](const Value&) {
    debug("writing close packet");
      std::vector<EngineIOPacket> packets;
      EngineIOPacket p;
      p.type = "close";
      packets.push_back(p);
      write(packets);
  };

    if (ReadyState::OPENED == _readyState) {
        debug("transport open - closing");
        close(Value::NONE);
    } else {
        // in case we're trying to close while
        // handshaking is in progress (GH-164)
        debug("transport not open - deferring close");
        once("open", close);
    }
}

bool EngineIOPolling::write(const std::vector<EngineIOPacket>& packets)
{
  _writable = false;
  auto callbackfn = [this](const Value&) {
    _writable = true;
    emit("drain");
  };

  const Value& data = engineio::parser::encodePayload(packets, _supportsBinary);
  doWrite(data, callbackfn);
    return true;
}

/**
 * Generates uri for connection.
 *
 * @api private
 */

//Polling.prototype.uri = function () {
//  var query = this.query || {};
//  var schema = this.secure ? "https' : 'http";
//  var port = "";
//
//  // cache busting is forced
//  if (false != this.timestampRequests) {
//    query[this.timestampParam] = yeast();
//  }
//
//  if (!this.supportsBinary && !query.sid) {
//    query.b64 = 1;
//  }
//
//  query = parseqs.encode(query);
//
//  // avoid port if default for schema
//  if (this.port && (("https" == schema && Number(this.port) != 443) ||
//     ("http" == schema && Number(this.port) != 80))) {
//    port = ":" + this.port;
//  }
//
//  // prepend ? to query
//  if (query.length) {
//    query = "?" + query;
//  }
//
//  var ipv6 = this.hostname.indexOf(":") != -1;
//  return schema + "://' + (ipv6 ? '[' + this.hostname + ']" : this.hostname) + port + this.path + query;
//};
