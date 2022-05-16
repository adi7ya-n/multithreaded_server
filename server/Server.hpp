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

// Handler to accept new connection, add to a pool, before starting a game.
class PlayerHandler : public boost::enable_shared_from_this<PlayerHandler>
{
    private:
        asio::io_service        &_service;
        tcp::socket              _socket;
        asio::io_service::strand _write_strand;
        bool                     _gameReady;
        bool                     _inQueue;
        string                   _userName;
        asio::streambuf          _in_packet;
        // std::stringstream       _streamBuffReader;
        // std::deque<std::string>  _send_packet_queue;

    public:
        PlayerHandler(asio::io_service &service)
            : _service(service), _socket(service), _write_strand(service),
              _gameReady(false), _inQueue(false)
        {
        }
        tcp::socket &socket()
        {
            return _socket;
        }
        void getUserName()
        {
            sendMsg(PacketType::CONN_PACKET, MsgType::USERNAME_REQUEST,
                    [this](err const &error, std::size_t bytes_transferred) {
                        SERVER_LOG(lg, DEBUG)
                            << "Successfully sent username request to ("
                            << _socket.remote_endpoint().address().to_string()
                            << " ,"
                            << std::to_string(_socket.remote_endpoint().port())
                            << ").";
                        asio::async_read(
                            _socket, _in_packet,
                            [this](err const  &error,
                                   std::size_t bytes_transferred) {
                                SERVER_LOG(lg, DEBUG)
                                    << "Received username from ("
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
            stream >> _userName;
            SERVER_LOG(lg, DEBUG) << _userName << " is ready to play.";
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
        unique_ptr<std::list<shared_ptr<PlayerHandler>>> _list;
        mutex                                            _listLock;

    public:
        TS_List()
        {
            _list = make_unique<std::list<shared_ptr<PlayerHandler>>>();
        }
        void insert(shared_ptr<PlayerHandler> handler)
        {
            const lock_guard<mutex> lock(_listLock);
            _list->push_back(handler);
        }
        std::list<shared_ptr<PlayerHandler>>::iterator
        remove(std::list<shared_ptr<PlayerHandler>>::iterator it)
        {
            const lock_guard<mutex> lock(_listLock);
            return _list->erase(it);
        }
        std::list<shared_ptr<PlayerHandler>> &data()
        {
            return *_list;
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
        shared_ptr<PlayerHandler> pop()
        {
            const lock_guard<mutex> lock(_queueLock);
            auto                    handler = first();
            _queue.pop();
            return handler;
        }
        shared_ptr<PlayerHandler> first()
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
        using shared_handler_t = shared_ptr<PlayerHandler>;

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
        void handleNewConnection(shared_handler_t handler, err const &error);
        // void newPlayer();
        void startGame(shared_handler_t player1, shared_handler_t player2);
        // void sendMessage();
};

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startServer(uint16_t port)
{
    auto handler = boost::make_shared<PlayerHandler>(_io_service);

    _port = port;

    initLogger("server.log", true);
    SERVER_LOG(lg, INFO) << "Initializing server. Port: " << _port;

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
            SERVER_LOG(lg, INFO) << "Worker thread spawned.";
            while (1)
            {
                _io_service.run();
            }
        });
    }

    thread incomingClientProcessor([&]() {
        while (1)
        {
            auto it = _clientHandlers.data().begin();
            while (it != _clientHandlers.data().end())
            {
                if ((*it)->gameReady())
                {
                    _gameReadyPlayers.insert(*it);
                    it = _clientHandlers.remove(it);
                }
                else
                {
                    it++;
                }
            }
        }
    });

    thread gameScheduler([&]() {
        while (1)
        {
            if (_gameReadyPlayers.size() >= 2)
            {
                auto handler1 = _gameReadyPlayers.pop();
                auto handler2 = _gameReadyPlayers.pop();
                startGame(handler1, handler2);
            }
        }
    });

    while (1)
    {
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::handleNewConnection(shared_handler_t handler, err const &error)
{
    auto new_handler = boost::make_shared<PlayerHandler>(_io_service);
    _acceptor.async_accept(new_handler->socket(), [=](auto ec) {
        handleNewConnection(new_handler, ec);
    });

    SERVER_LOG(lg, INFO)
        << "Incoming connection from ("
        << handler->socket().remote_endpoint().address().to_string() << ", "
        << handler->socket().remote_endpoint().port() << ")";
    ;
    if (error)
    {
        SERVER_LOG(lg, ERR)
            << "Error while trying to handle new connection. Error: "
            << error.message();
        return;
    }

    _clientHandlers.insert(handler);

    handler->getUserName();
    // handler->sendMsg(PacketType::CONN_PACKET, MsgType::USERNAME_REQUEST);
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void Server::startGame(shared_handler_t player1, shared_handler_t player2)
{
    SERVER_LOG(lg, INFO) << "Starting game between " << player1->userName()
                         << " and " << player2->userName();
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#endif