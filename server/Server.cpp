#include "Server.hpp"

bool Server::startServer(uint16_t port)
{
    _port = port;
    initLogger("server.log", true);
    SERVER_LOG(lg, TRACE) << "Initializing server. Port: " << _port;
    flushLogs();
    return true;
}
