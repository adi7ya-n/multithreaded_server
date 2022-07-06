#ifndef GAME_HPP
#define GAME_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <bitset>
#include <thread>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <functional>
#include <utility>
#include <bits/stdc++.h>
#include <algorithm>
#include <array>
#include <queue>
#include <sstream>
#include <list>

#include "Logger.hpp"

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

namespace asio = boost::asio;

using tcp = boost::asio::ip::tcp;
using err = boost::system::error_code;

using std::unique_ptr, std::make_unique, std::vector, std::thread, std::mutex,
    std::lock_guard, std::pair, std::make_pair, std::remove_if, std::array,
    std::atomic, std::queue, std::bitset, std::string, std::cout,
    boost::weak_ptr, std::endl;

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

namespace GameLib
{
    enum PacketType : uint8_t
    {
        CONN_PACKET  = 170,
        DATA_PACKET  = 255,
        ADMIN_PACKET = 204
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum ConnMsg : uint8_t
    {
        USERNAME_REQUEST,
        NUM_OF_GAMES,
        REBOOT_SERVER,
        DISPLAY_ONGOING_GAMES,
        GET_GAME_INFO,
        PLAYER1_INDICATION,
        PLAYER2_INDICATION,
        START_SERVER,
        SHUTDOWN_SERVER
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum Move : uint8_t
    {
        ONE = 1,
        TWO,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    struct Packet
    {
            uint8_t type_;
            uint8_t data_;

            Packet(uint8_t type, uint8_t data)
            {
                type_ = type;
                data_ = data;
            }

            Packet()
            {
                type_ = 0;
                data_ = 0;
            }

            static Packet create(uint8_t type, uint8_t data)
            {
                return Packet(type, data);
            }
    } __attribute__((packed));

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum GameResult : uint8_t
    {
        DRAW      = 11,
        O_WIN     = 12,
        X_WIN     = 13,
        NO_RESULT = 14
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    inline const string to_string(GameResult result)
    {
        switch (result)
        {
            case GameResult::DRAW:
                return "DRAW";
            case GameResult::X_WIN:
                return "X_WIN";
            case GameResult::O_WIN:
                return "O_WIN";
        }
    }

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    inline const string to_string(Packet packet)
    {
        string result;

        if (packet.type_ == PacketType::CONN_PACKET)
        {
            result += "CONN_PACKET::";
            switch (packet.data_)
            {
                case ConnMsg::USERNAME_REQUEST:
                    return result + "USERNAME_REQUEST";
                case ConnMsg::NUM_OF_GAMES:
                    return result + "NUM_OF_GAMES";
                case ConnMsg::REBOOT_SERVER:
                    return result + "REBOOT_SERVER";
                case ConnMsg::DISPLAY_ONGOING_GAMES:
                    return result + "DISPLAY_ONGOING_GAMES";
                case ConnMsg::GET_GAME_INFO:
                    return result + "GET_GAME_INFO";
                case ConnMsg::PLAYER1_INDICATION:
                    return result + "PLAYER1_INDICATION";
                case ConnMsg::PLAYER2_INDICATION:
                    return result + "PLAYER2_INDICATION";
                default:
                    return "INVALID_CONN_PACKET_DATA";
            }
        }
        else
        {
            result += "DATA_PACKET::";
            switch (packet.data_)
            {
                case Move::ONE:
                    return result + "ONE";
                case Move::TWO:
                    return result + "TWO";
                case Move::THREE:
                    return result + "THREE";
                case Move::FOUR:
                    return result + "FOUR";
                case Move::FIVE:
                    return result + "FIVE";
                case Move::SIX:
                    return result + "SIX";
                case Move::SEVEN:
                    return result + "SEVEN";
                case Move::EIGHT:
                    return result + "EIGHT";
                case Move::NINE:
                    return result + "NINE";
                case GameResult::X_WIN:
                    return result + "X_WIN";
                case GameResult::O_WIN:
                    return result + "O_WIN";
                case GameResult::DRAW:
                    return result + "DRAW";
            }
        }
        return "INVALID_PACKET_TYPE";
    }

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    // Handler to accept new connection, send and receive messages.
    class PlayerHandler : public std::enable_shared_from_this<PlayerHandler>
    {
        private:
            asio::io_service &service_;
            tcp::socket       socket_;
            bool              gameReady_;
            string            userName_;
            asio::streambuf   inputStreamBuf_;
            std::istream      inputStream_;
            vector<uint8_t>   inputBuffer_;
            vector<uint8_t>   outputBuffer_;

        public:
            PlayerHandler(asio::io_service &service)
                : service_(service), socket_(service), gameReady_(false),
                  inputStream_(&inputStreamBuf_)
            {
                inputBuffer_.resize(sizeof(Packet));
                outputBuffer_.resize(sizeof(Packet));
            }

            tcp::socket &socket()
            {
                return socket_;
            }

            void setUserName()
            {
                std::getline(inputStream_, userName_);
                gameReady_ = true;
            }

            bool gameReady()
            {
                return (gameReady_ == true);
            }

            string &userName()
            {
                return userName_;
            }

            void sendMsg(Packet                                packet,
                         std::function<void(err, std::size_t)> cb)
            {
                LOG_INF << "Sending packet: " << to_string(packet);
                outputBuffer_[0] = packet.type_;
                outputBuffer_[1] = packet.data_;
                asio::async_write(
                    socket_, asio::buffer(outputBuffer_, sizeof(Packet)), cb);
            }

            void readString(std::function<void(err, std::size_t)> cb)
            {
                asio::async_read_until(socket_, inputStreamBuf_, '\n', cb);
            }

            void readMove(std::function<void(err, std::size_t)> cb)
            {
                asio::async_read(socket_, asio::buffer(inputBuffer_),
                                 asio::transfer_exactly(sizeof(Packet)), cb);
            }

            uint8_t getMove()
            {
                Packet movePacket;
                movePacket.type_ = inputBuffer_[0];
                movePacket.data_ = inputBuffer_[1];

                // TODO: Error handling
                return movePacket.data_;
            }

            void getUserName()
            {
                sendMsg(
                    Packet::create(PacketType::CONN_PACKET,
                                   ConnMsg::USERNAME_REQUEST),
                    [this](err const &error, std::size_t bytes_transferred) {
                        LOG_DBG
                            << "Successfully sent username request("
                            << bytes_transferred << " bytes) to ("
                            << socket_.remote_endpoint().address().to_string()
                            << " ,"
                            << std::to_string(socket_.remote_endpoint().port())
                            << ").";

                        readString([this](err const  &error,
                                          std::size_t bytes_transferred) {
                            if (error)
                            {
                                LOG_ERR << "Error during reception of "
                                           "username: "
                                        << error.message();
                            }
                            else
                            {
                                LOG_DBG << "Received username from ("
                                        << socket_.remote_endpoint()
                                               .address()
                                               .to_string()
                                        << " ,"
                                        << std::to_string(
                                               socket_.remote_endpoint().port())
                                        << ")."
                                        << "read: " << bytes_transferred
                                        << " bytes.";
                                setUserName();
                            }
                        });
                    });
            }
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum PlayerIdentifer : uint8_t
    {
        O = 0,
        X = 1
    };

    class Game
    {
            using Board = std::array<std::array<uint8_t, 3>, 3>;

        private:
            Board                          board_;
            std::shared_ptr<PlayerHandler> player1_, player2_;
            uint8_t                        gameId_;
            uint8_t                        moveCount_;
            GameResult                     gameResult_;
            bool                           gameOver_;

        public:
            static constexpr uint8_t EMPTY              = 2;
            static constexpr uint8_t MAX_POSSIBLE_MOVES = 9;
            Game(std::shared_ptr<PlayerHandler> &player1,
                 std::shared_ptr<PlayerHandler> &player2)
            {
                player1_ = std::move(player1);
                player2_ = std::move(player2);
                for (auto &row : board_)
                {
                    std::fill(row.begin(), row.end(), EMPTY);
                }
                moveCount_ = 0;
                gameOver_  = false;
                setup();
            }
            ~Game()
            {
                player1_->socket().close();
                player2_->socket().close();
                LOG_DBG << "~Game called.";
            }
            void setup();
            void start();
            void readMove(PlayerIdentifer id);
            void sendMove(PlayerIdentifer id, uint8_t move, bool finalMove);
            void updateBoard(PlayerIdentifer id, uint8_t move);
            void updateBoardAndCheckResult(PlayerIdentifer id, uint8_t move);
            void sendResultToPlayers(uint8_t move);
            GameResult checkResult();
            bool       gameOver();
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

} // namespace GameLib

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#endif