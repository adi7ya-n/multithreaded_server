#ifndef SERVER_HPP
#define SERVER_HPP

#include "Game.hpp"

using namespace Logging;
using namespace GameLib;

constexpr uint16_t DEFAULT_PORT           = 9000;
constexpr uint16_t THREAD_COUNT           = 5;
constexpr uint16_t MAXIMUM_NUM_OF_PLAYERS = 10000;

template <class T> class TS_List
{
    private:
        std::list<std::shared_ptr<T>> list_;
        mutex                         listLock_;

    public:
        using listIter = typename std::list<std::shared_ptr<T>>::iterator;

        TS_List()
        {
        }

        void insert(std::shared_ptr<T> handler)
        {
            const lock_guard<mutex> lock(listLock_);
            list_.push_back(handler);
        }

        listIter remove(listIter it)
        {
            const lock_guard<mutex> lock(listLock_);
            return list_.erase(it);
        }

        listIter begin()
        {
            return list_.begin();
        }

        listIter end()
        {
            return list_.end();
        }
        std::shared_ptr<T> back()
        {
            return list_.back();
        }
};

class TS_Queue
{
    private:
        queue<std::shared_ptr<PlayerHandler>> queue_;
        mutex                                 queueLock_;

    public:
        TS_Queue()
        {
        }
        void insert(std::shared_ptr<PlayerHandler> handler)
        {
            const lock_guard<mutex> lock(queueLock_);
            queue_.push(handler);
        }
        std::weak_ptr<PlayerHandler> pop()
        {
            const lock_guard<mutex> lock(queueLock_);
            auto                    handler = first();
            queue_.pop();
            return handler;
        }
        std::weak_ptr<PlayerHandler> first()
        {
            // const lock_guard<mutex> lock(queueLock_);
            return queue_.front();
        }
        bool empty()
        {
            return queue_.empty();
        }
        int size()
        {
            return static_cast<int>(queue_.size());
        }
};

class Server
{
    private:
        uint16_t               port_;
        uint16_t               threadCount_;
        vector<thread>         threadPool_;
        asio::io_service       io_service_;
        tcp::acceptor          acceptor_;
        TS_List<PlayerHandler> clientHandlers_;
        TS_List<Game>          runningGames_;
        volatile bool          shutDownCommand_;

    public:
        Server(uint16_t threadCount = 1)
            : threadCount_(threadCount), acceptor_(io_service_),
              shutDownCommand_(false)
        {
        }

        void startServer(uint16_t port);
        void handleNewConnection(const std::shared_ptr<PlayerHandler> &handler,
                                 err const                            &error);
        void startGame(std::shared_ptr<PlayerHandler> &player1,
                       std::shared_ptr<PlayerHandler> &player2);
        void startClientProcessor();
};

#endif