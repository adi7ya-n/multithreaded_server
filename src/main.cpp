#include "Server.hpp"

int main(int argc, char *argv[])
{
    uint16_t           port   = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_PORT;
    unique_ptr<Server> server = make_unique<Server>(THREAD_COUNT);
    server->startServer(port);
    flushLogs();
    return 0;
}