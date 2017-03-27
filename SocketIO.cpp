#include "SocketIO.h"

#include "SocketIOManager.h"
#include "SocketIOParser.h"

/**
 * Managers cache.
 */

static std::unordered_map<std::string, SocketIOSocket*> __cache;

std::shared_ptr<SocketIOSocket> SocketIO::connect(const std::string& uri, const Opts& opts)
{
//  opts = opts || {};

//  var parsed = url(uri);
//  std::string source = parsed.source;
//  std::string id = parsed.id;
//  std::string path = parsed.path;
//  bool foundIdInCache = __cache.find(id) != __cache.end();
//  bool sameNamespace = foundIdInCache && __cache[id]._nsps.find(path) != __cache[id]._nsps.end();
//  bool newConnection = opts.forceNew || opts["force new connection"] ||
//                      false == opts.multiplex || sameNamespace;
//
//  std::shared_ptr<SocketIOSocket> io;
//
//  if (newConnection) {
//    debug("ignoring socket cache for %s", source.c_str());
//    io = new SocketIOManager(source, opts);
//  } else {
//    if (!foundIdInCache) {
//      debug("new io instance for %s", source.c_str());
//      __cache.emplace(id, new SocketIOManager(source, opts));
//    }
//    io = __cache[id];
//  }
//
//  if (parsed.query && !opts.query) {
//    opts.query = parsed.query;
//  } else if (opts && "object" == typeof opts.query) {
//    opts.query = encodeQueryString(opts.query);
//  }
//  return io->createSocket(parsed.path, opts);
    return nullptr;
}
/**
 *  Helper method to parse query objects to string.
 * @param {object} query
 * @returns {string}
 */
//cjh function encodeQueryString (obj) {
//   var str = [];
//   for (var p in obj) {
//     if (obj.hasOwnProperty(p)) {
//       str.push(encodeURIComponent(p) + "=" + encodeURIComponent(obj[p]));
//     }
//   }
//   return str.join("&");
// }

int SocketIO::getProtocolVersion()
{
    return socketio::parser::getProtocolVersion();
}

