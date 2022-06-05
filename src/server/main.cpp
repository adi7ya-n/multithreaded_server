#include "Server.hpp"

int main()
{
    unique_ptr<Server> server = make_unique<Server>(THREAD_COUNT);
    server->startServer(9000);
    flushLogs();
    return 0;
}