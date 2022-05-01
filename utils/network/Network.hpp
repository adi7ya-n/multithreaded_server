#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "Game.hpp"

using std::vector, std::cout, std::unique_ptr;
using namespace Game;

namespace Network
{
    class Session
    {
            uint32_t _sessionId;
            Player   _player1, _player2;
            Board    _board;

        public:
            Session()
            {
                _sessionId = 0u;
                _player1.setIdentity(PlayerIdentifier::X);
                _player2.setIdentity(PlayerIdentifier::O);
            }
    };

    enum class PacketType : uint8_t
    {
        CONN_PACKET  = 0,
        DATA_PACKET  = 1,
        ADMIN_PACKET = 2
    };

    enum ConnectType : uint8_t
    {
        CONN_HOST_ALIVE = 0,
        CONN_REQUEST    = 1,
        CONN_ACCEPT     = 2,
        CONN_REJECT     = 3
    };

    enum AdminType : uint8_t
    {
        NUM_OF_GAMES          = 0,
        REBOOT_SERVER         = 1,
        DISPLAY_ONGOING_GAMES = 2,
        GET_GAME_INFO         = 3
    };

    struct Packet
    {
            PacketType _type;
            uint8_t    _data;

        public:
            bool createPkt(PacketType msgType, uint8_t data)
            {
                switch (msgType)
                {
                    case PacketType::CONN_PACKET:
                    case PacketType::DATA_PACKET:
                    case PacketType::ADMIN_PACKET:
                        _type = msgType;
                        break;
                    default:
                        cout << "Invalid packet type: " << msgType << "\n";
                        return false;
                }
                if (_type == PacketType::CONN_PACKET)
                {
                    if (data >= CONN_HOST_ALIVE and data <= CONN_REJECT)
                    {
                        _data = data;
                    }
                    else
                    {
                        cout << "Invalid connection message payload: " << data
                             << "\n";
                        return false;
                    }
                }
                else if (_type == PacketType::DATA_PACKET)
                {
                    if (data >= 0 and data <= 9)
                    {
                        _data = data;
                    }
                    else
                    {
                        cout << "Invalid data message payload: " << data
                             << "\n";
                        return false;
                    }
                }
                else
                {
                    if (data >= NUM_OF_GAMES and data <= DISPLAY_ONGOING_GAMES)
                    {
                        _data = data;
                    }
                    else
                    {
                        cout << "Invalid admin message payload: " << data
                             << "\n";
                    }
                }
                return true;
            }
    };

    class Server
    {
            vector<unique_ptr<Session>> _sessions;

        public:
            void StartServer(uint16_t port);
            void RestartServer();
    };
} // namespace Network

#endif