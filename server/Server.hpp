#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/Utilities.hpp"
#include "../utils/logger/Logger.hpp"

using std::unique_ptr, std::make_unique, std::vector, std::thread;
using namespace Logging;

namespace asio = boost::asio;
// namespace system = boost::system;
// namespace ip = boost::asio::ip;

using tcp = boost::asio::ip::tcp;
using err = boost::system::error_code;

// Handler to accept new connection, add to a pool, before starting a game.
class UserHandler : public boost::enable_shared_from_this<UserHandler>
{
    private:
        asio::io_service        &_service;
        tcp::socket              _socket;
        asio::io_service::strand _write_strand;
        asio::streambuf          _in_packet;
        std::deque<std::string>  _send_packet_queue;

    public:
        UserHandler(asio::io_service &service)
            : _service(service), _socket(service), _write_strand(service)
        {
        }

        tcp::socket &socket()
        {
            return _socket;
        }
        void start()
        {
            SERVER_LOG(lg, INFO) << "Entered start function";
        }
};

template <typename ConnectionHandler> class Server
{
        using shared_handler_t = shared_ptr<ConnectionHandler>;

    private:
        uint16_t         _port;
        uint8_t          _thread_count;
        vector<thread>   _thread_pool;
        asio::io_service _io_service;
        tcp::acceptor    _acceptor;

    public:
        Server(uint8_t thread_count = 1)
            : _thread_count(thread_count), _acceptor(_io_service)
        {
        }

        void startServer(uint16_t port);
        void handleNewConnection(shared_handler_t handler, err const &error);
};

template <class T> void Server<T>::startServer(uint16_t port)
{
    auto handler = boost::make_shared<T>(_io_service);

    _port = port;

    // SERVER_LOG(lg, INFO) << "Initializing server. Port: " << _port;

    initLogger("server.log", true);
    SERVER_LOG(lg, INFO) << "Initializing server on port: " << _port;

    tcp::endpoint endpoint(tcp::v4(), _port);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    _acceptor.async_accept(handler->socket(),
                           [=](auto ec) { handleNewConnection(handler, ec); });
    for (auto i = 0; i < _thread_count; ++i)
    {
        _thread_pool.emplace_back([=] {
            SERVER_LOG(lg, INFO) << "Worker thread spawned.";
            while (1)
            {
                // cout << "Thread " << std::this_thread::get_id() << "Running\n";
                _io_service.run();
            }
        });
    }
    SERVER_LOG(lg, INFO) << "All worker threads spawned.";
    while (1);
}

template <class T>
void Server<T>::handleNewConnection(shared_handler_t handler, err const &error)
{

    SERVER_LOG(lg, INFO) << "Entered handleNewConnection";
    if (error)
    {
        SERVER_LOG(lg, ERR)
            << "Error while trying to handle new connection. Error: "
            << error.message();
        return;
    }
    handler->start();
    auto new_handler = boost::make_shared<T>(_io_service);
    _acceptor.async_accept(new_handler->socket(), [=](auto ec) {
        handleNewConnection(new_handler, ec);
    });

    SERVER_LOG(lg, INFO) << "Leaving handleNewConnection";
}

#endif