#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/Utilities.hpp"
#include "../utils/logger/Logger.hpp"

using std::unique_ptr, std::make_unique;
using namespace Logging;
class Server
{
        uint16_t _port;

    public:
        bool startServer(uint16_t port);
        
};

#endif