#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "../../game/Game.hpp"
#include "../Utilities.hpp"

using namespace Game;
namespace Network
{
    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum class PacketType : uint8_t
    {
        CONN_PACKET  = 0xAA,
        DATA_PACKET  = 0xBB,
        ADMIN_PACKET = 0xCC
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    enum MsgType : uint8_t
    {
        USERNAME_REQUEST,
        NUM_OF_GAMES,
        REBOOT_SERVER,
        DISPLAY_ONGOING_GAMES,
        GET_GAME_INFO,
        PLAYER1_INDICATION,
        PLAYER2_INDICATION
    };

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    string to_string(PacketType type)
    {
        switch (type)
        {
            case PacketType::CONN_PACKET:
                return "CONN_PACKET";
            case PacketType::ADMIN_PACKET:
                return "ADMIN_PACKET";
            case PacketType::DATA_PACKET:
                return "DATA_PACKET";
            default:
                return "INVALID_PACKET_TYPE";
        }
    }

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    string to_string(uint8_t data)
    {
        switch (data)
        {
            case MsgType::USERNAME_REQUEST:
                return "USERNAME_REQUEST";
            case MsgType::NUM_OF_GAMES:
                return "NUM_OF_GAMES";
            case MsgType::REBOOT_SERVER:
                return "REBOOT_SERVER";
            case MsgType::DISPLAY_ONGOING_GAMES:
                return "DISPLAY_ONGOING_GAMES";
            case MsgType::GET_GAME_INFO:
                return "GET_GAME_INFO";
            case MsgType::PLAYER1_INDICATION:
                return "PLAYER1_INDICATION";
            case MsgType::PLAYER2_INDICATION:
                return "PLAYER2_INDICATION";
            default:
                return "INVALID_DATA";
        }
    }

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    bool createPkt(vector<uint8_t> &buffer, PacketType type, uint8_t data)
    {
        if (type == PacketType::CONN_PACKET)
        {
            switch (data)
            {
                case MsgType::USERNAME_REQUEST:
                    break;
                case MsgType::PLAYER1_INDICATION:
                    break;
                case MsgType::PLAYER2_INDICATION:
                    break;
                default:
                    SERVER_LOG(lg, ERR) << "Packet error: " << to_string(type)
                                        << " " << to_string(data);
                    return false;
            }
        }
        else if (type == PacketType::ADMIN_PACKET)
        {
            switch (data)
            {
                case MsgType::NUM_OF_GAMES:
                case MsgType::REBOOT_SERVER:
                case MsgType::DISPLAY_ONGOING_GAMES:
                case MsgType::GET_GAME_INFO:
                    break;
                default:
                    SERVER_LOG(lg, ERR) << "Packet error: " << to_string(type)
                                        << " " << to_string(data);
                    return false;
            }
        }
        else if (type == PacketType::DATA_PACKET)
        {
            switch (data)
            {
                case PlayerIdentifier::X:
                case PlayerIdentifier::O:
                    break;
                default:
                    SERVER_LOG(lg, ERR) << "Packet error: " << to_string(type)
                                        << " " << to_string(data);
                    return false;
            }
        }
        else
        {
            SERVER_LOG(lg, ERR) << "Packet error: " << to_string(type) << " "
                                << to_string(data);
            return false;
        }
        buffer.push_back(static_cast<uint8_t>(type));
        buffer.push_back(data);
        return true;
    }

    /*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/
}; // namespace Network

#endif