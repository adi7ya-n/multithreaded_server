#include "Server.hpp"

int main()
{
    unique_ptr<Server<UserHandler>> server = make_unique<Server<UserHandler>>(5);
    cout << "Starting server\n";
    server->startServer(9000);
    cout << "After startServer\n";
    flushLogs();
    return 0;
}