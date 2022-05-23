#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/Utilities.hpp"
#include "../utils/logger/Logger.hpp"
#include "../utils/network/Network.hpp"

#define THREAD_COUNT           5
#define MAXIMUM_NUM_OF_PLAYERS 10000

using namespace Logging;
using namespace Network;

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

// Handler to accept new connection, send and receive messages.
class PlayerHandler : public boost::enable_shared_from_this<PlayerHandler>
{
    private:
        asio::io_service        &_service;
        tcp::socket              _socket;
        asio::io_service::strand _write_strand;
        bool                     _gameReady;
        bool                     _inQueue;
        bool                     _inGame;
        string                   _userName;
        asio::streambuf          _in_packet;

    public:
        PlayerHandler(asio::io_service &service)
            : _service(service), _socket(service), _write_strand(service),
              _gameReady(false), _inQueue(false), _inGame(false)
        {
        }
        tcp::socket &socket()
        {
            return _socket;
        }
        void getUserName()
        {
            sendMsg(
                PacketType::CONN_PACKET, MsgType::USERNAME_REQUEST,
                [this](err const &error, std::size_t bytes_transferred) {
                    LOG_DBG << "Successfully sent username request to ("
                            << _socket.remote_endpoint().address().to_string()
                            << " ,"
                            << std::to_string(_socket.remote_endpoint().port())
                            << ").";
                    asio::async_read_until(
                        _socket, _in_packet, '\n',
                        [this](err const  &error,
                               std::size_t bytes_transferred) {
                            if (error)
                            {
                                LOG_ERR
                                    << "Error during reception of username: "
                                    << error.message();
                            }
                            LOG_DBG << "Received username from ("
                                    << _socket.remote_endpoint()
                                           .address()
                                           .to_string()
                                    << " ,"
                                    << std::to_string(
                                           _socket.remote_endpoint().port())
                                    << ")."
                                    << "read: " << bytes_transferred
                                    << " bytes.";
                            setUserName();
                        });
                });
        }
        void sendMsg(PacketType msg, uint8_t data,
                     std::function<void(err, std::size_t)> cb)
        {
            auto buffer = boost::make_shared<vector<uint8_t>>();
            if (createPkt(*buffer, msg, data))
            {
                auto successMsg = boost::make_shared<string>();
                *successMsg =
                    to_string(msg) + "::" + to_string(data) + " sent to: (" +
                    _socket.remote_endpoint().address().to_string() + ", " +
                    std::to_string(_socket.remote_endpoint().port()) + ")";

                asio::async_write(_socket, asio::buffer(*buffer), cb);
            }
        }
        void setUserName()
        {
            std::istream stream(&_in_packet);
            std::getline(stream, _userName);
            LOG_DBG << _userName << " is ready to play.";
            _gameReady = true;
        }
        bool inQueue()
        {
            return (_inQueue == true);
        }
        bool gameReady()
        {
            return (_gameReady == true);
        }
        bool inGame()
        {
            return (_inGame == true);
        }
        void setInGame(bool inGame)
        {
            _inGame = inGame;
        }
        string &userName()
        {
            return _userName;
        }
        asio::streambuf &streamBuffer()
        {
            return _in_packet;
        }
};

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

class TS_List
{
    private:
        std::list<shared_ptr<PlayerHandler>> _list;
        mutex                                _listLock;

    public:
        TS_List()
        {
        }
        void insert(shared_ptr<PlayerHandler> handler)
        {
            const lock_guard<mutex> lock(_listLock);
            _list.push_back(handler);
        }
        std::list<shared_ptr<PlayerHandler>>::iterator
        remove(std::list<shared_ptr<PlayerHandler>>::iterator it)
        {
            const lock_guard<mutex> lock(_listLock);
            return _list.erase(it);
        }
        std::list<shared_ptr<PlayerHandler>> &data()
        {
            return _list;
        }
};

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

// Thread-safe queue
class TS_Queue
{
    private:
        queue<shared_ptr<PlayerHandler>> _queue;
        mutex                            _queueLock;

    public:
        TS_Queue()
        {
        }
        void insert(shared_ptr<PlayerHandler> handler)
        {
            const lock_guard<mutex> lock(_queueLock);
            _queue.push(handler);
        }
        weak_ptr<PlayerHandler> pop()
        {
            const lock_guard<mutex> lock(_queueLock);
            auto                    handler = first();
            _queue.pop();
            return handler;
        }
        weak_ptr<PlayerHandler> first()
        {
            // const lock_guard<mutex> lock(_queueLock);
            return _queue.front();
        }
        bool empty()
        {
            return _queue.empty();
        }
        int size()
        {
            return static_cast<int>(_queue.size());
        }
};

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

class Server
{
    private:
        uint16_t         _port;
        uint8_t          _thread_count;
        vector<thread>   _thread_pool;
        asio::io_service _io_service;
        tcp::acceptor    _acceptor;
        TS_List          _clientHandlers;
        TS_Queue         _gameReadyPlayers;

    public:
        Server(uint8_t thread_count = 1)
            : _thread_count(thread_count), _acceptor(_io_service)
        {
        }

        void startServer(uint16_t port);
        void handleNewConnection(shared_ptr<PlayerHandler> handler,
                                 err const                &error);
        void startGame(shared_ptr<PlayerHandler> &player1,
                       shared_ptr<PlayerHandler> &player2);
};

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startServer(uint16_t port)
{
    auto handler = boost::make_shared<PlayerHandler>(_io_service);

    _port = port;

    initLogger("server.log", true);
    LOG_INF << "Initializing server. Port: " << _port;

    tcp::endpoint endpoint(tcp::v4(), _port);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    _acceptor.async_accept(handler->socket(),
                           [=](auto ec) { handleNewConnection(handler, ec); });
    for (auto i = 0; i < _thread_count; ++i)
    {
        _thread_pool.emplace_back([&] {
            LOG_INF << "Worker thread spawned.";
            while (1)
            {
                _io_service.run();
            }
        });
    }

    thread incomingClientProcessor([&]() {
        while (1)
        {
            auto end = _clientHandlers.data().end();
            auto it  = _clientHandlers.data().begin();

            while (it != end)
            {
                if ((*it)->gameReady() and !(*it)->inGame())
                {
                    shared_ptr<PlayerHandler> player1 = *it;
                    it++;
                    while (it != end)
                    {
                        if ((*it)->gameReady() and !(*it)->inGame())
                        {
                            startGame(player1, *it);
                            it++;
                            break;
                        }
                        else
                            it++;
                    }
                }
                else
                    it++;
            }
        }
    });

    while (1)
    {
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::handleNewConnection(shared_ptr<PlayerHandler> handler,
                                 err const                &error)
{
    auto new_handler = boost::make_shared<PlayerHandler>(_io_service);
    _acceptor.async_accept(new_handler->socket(), [=](auto ec) {
        handleNewConnection(new_handler, ec);
    });

    LOG_INF << "Incoming connection from ("
            << handler->socket().remote_endpoint().address().to_string() << ", "
            << handler->socket().remote_endpoint().port() << ")";

    if (error)
    {
        LOG_ERR << "Error while trying to handle new connection. Error: "
                << error.message();
        return;
    }

    _clientHandlers.insert(handler);
    handler->getUserName();
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startGame(shared_ptr<PlayerHandler> &player1,
                       shared_ptr<PlayerHandler> &player2)
{
    LOG_INF << "Starting game between " << player1->userName() << " and "
            << player2->userName();

    LOG_INF << "player1.use_count: " << player1.use_count() << " "
            << "Player2.use_count: " << player2.use_count();

    player1->setInGame(true);
    player2->setInGame(true);
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#endif