#include "Server.hpp"

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startClientProcessor()
{
    while (!shutDownCommand_)
    {
        auto end = clientHandlers_.end();
        auto it  = clientHandlers_.begin();

        while (it != end)
        {
            if ((*it)->gameReady())
            {
                auto player1 = it;
                it++;
                while (it != end)
                {
                    if ((*it)->gameReady())
                    {
                        startGame(*player1, *it);
                        clientHandlers_.remove(player1);
                        it = clientHandlers_.remove(it);
                        break;
                    }
                    else
                        it++;
                }
            }
            else
                it++;
        }

        auto gameIter = runningGames_.begin();
        auto lastGame = runningGames_.end();

        while (gameIter != lastGame)
        {
            if ((*gameIter)->gameOver())
            {
                (*gameIter).reset();
                gameIter = runningGames_.remove(gameIter);
            }
        }
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startServer(uint16_t port)
{
    port_ = port;

    clientHandlers_.insert(std::make_shared<PlayerHandler>(io_service_));

    initLogger("server.log", true);
    LOG_INF << "Initializing server on port: " << port_;

    tcp::endpoint endpoint(tcp::v4(), port_);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    acceptor_.async_accept(clientHandlers_.back()->socket(), [=](auto ec) {
        handleNewConnection(clientHandlers_.back(), ec);
    });

    for (auto i = 0; i < threadCount_; ++i)
    {
        threadPool_.emplace_back([&] {
            LOG_INF << "Worker thread spawned.";
            while (!shutDownCommand_)
            {
                io_service_.run();
            }
        });
    }
    startClientProcessor();
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::handleNewConnection(const std::shared_ptr<PlayerHandler> &handler,
                                 err const                            &error)
{
    handler->getUserName(); // TODO: Replace this with a login system.

    clientHandlers_.insert(std::make_shared<PlayerHandler>(io_service_));

    acceptor_.async_accept(clientHandlers_.back()->socket(), [=](auto ec) {
        handleNewConnection(clientHandlers_.back(), ec);
    });

    LOG_INF << "Incoming connection from ("
            << handler->socket().remote_endpoint().address().to_string() << ", "
            << handler->socket().remote_endpoint().port() << ")";

    if (error)
    {
        LOG_ERR << "Error while trying to handle new connection: "
                << error.message();
        return;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startGame(std::shared_ptr<PlayerHandler> &player1,
                       std::shared_ptr<PlayerHandler> &player2)
{
    runningGames_.insert(std::make_shared<Game>(player1, player2));
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
