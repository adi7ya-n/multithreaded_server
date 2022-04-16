#pragma once

#include <iostream>

using std::bitset;

namespace Game
{
    enum PlayerIdentifier : uint8_t
    {
        X = 1,
        O = 0
    };
    enum Result : uint8_t
    {
        NO_RESULT = 0,
        O_WINS    = 1,
        X_WINS    = 2
    };
    class Board
    {
            // 2D bitset
            bitset<9> board;

        public:
            bool operator()(int j, int k)
            {
                return (board[j + k]);
            }
            void setPosition(int j, int k, PlayerIdentifier player)
            {
                board[j + k] = player;
            }
    };

    class Player
    {
            PlayerIdentifier identifier;

        public:
            Player()
            {
                identifier = PlayerIdentifier::X;
            }
            void setIdentity(PlayerIdentifier id)
            {
                identifier = id;
            }
    };
} // namespace Game