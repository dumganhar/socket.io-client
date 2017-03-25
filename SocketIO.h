#pragma once

class SocketIO
{
public:
    /**
     * Looks up an existing `Manager` for multiplexing.
     * If the user summons:
     *
     *   `io("http://localhost/a");`
     *   `io("http://localhost/b");`
     *
     * We reuse the existing instance based on same scheme/port/host,
     * and we initialize sockets for each namespace.
     *
     * @api public
     */

    static SocketIOSocket* connect(const std::string& uri, const Opts& opts);

    /**
     * Protocol version.
     *
     * @api public
     */
    static int getProtocolVersion() const;
};