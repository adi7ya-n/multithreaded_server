#include "Server.hpp"

int main()
{
    unique_ptr<Server> server = make_unique<Server>();
    cout << "Starting server\n";
    server->startServer(9000);
    return 0;
}