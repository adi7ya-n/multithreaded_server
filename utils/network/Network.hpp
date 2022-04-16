#pragma once

#include "Game.hpp"
#include <bitset>
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

using std::vector, std::bitset, std::cout;
using namespace Game;

namespace Network
{
    class Session
    {
            uint32_t sessionId;
            Player player1, player2;
            Board board;

        public:
            Session()
            {
                sessionId = 0u;
                player1.setIdentity(PlayerIdentifier::X);
                player2.setIdentity(PlayerIdentifier::O);
            }
    };

    enum PacketType : uint8_t
    {
        CONN_PACKET = 0,
        DATA_PACKET = 1
    };

    enum ConnectType : uint8_t
    {
        CONN_HOST_ALIVE = 0,
        CONN_REQUEST    = 1,
        CONN_ACCEPT     = 2,
        CONN_REJECT     = 3
    };

    struct Packet
    {
            PacketType type;
            uint8_t data;

        public:
            bool createPkt(PacketType msgType, uint8_t data)
            {
                switch (msgType)
                {
                    case CONN_PACKET:
                    case DATA_PACKET:
                        type = msgType;
                        break;
                    default:
                        cout << "Invalid packet type: " << msgType << "\n";
                        return false;
                }
                if (type == CONN_PACKET)
                {
                    if (data >= CONN_HOST_ALIVE and data <= CONN_REJECT)
                    {
                        this->data = data;
                    }
                    else
                    {
                        cout << "Invalid connection message payload: " << data
                             << "\n";
                        return false;
                    }
                }
                else
                {
                    if (data >= 0 and data <= 9)
                    {
                        this->data = data;
                    }
                    else
                    {
                        cout << "Invalid data message payload: " << data
                             << "\n";
                        return false;
                    }
                }
                return true;
            }
    };

    class Server
    {
            vector<Session> sessions;

        public:
            void StartServer();
    };
} // namespace Network
