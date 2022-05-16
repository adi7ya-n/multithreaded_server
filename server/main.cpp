#include "Server.hpp"

int main()
{
    unique_ptr<Server> server = make_unique<Server>(THREAD_COUNT);
    cout << "Starting server\n";
    cout<< "Size of playerHandler: " << sizeof(PlayerHandler) << std::endl;
    server->startServer(9000);
    cout << "After startServer\n";
    flushLogs();
    return 0;
}