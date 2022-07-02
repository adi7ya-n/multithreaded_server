#include "Game.hpp"

namespace GameLib
{
    void Game::setup()
    {
        player1_->sendMsg(
            Packet::create(PacketType::CONN_PACKET,
                           ConnMsg::PLAYER1_INDICATION),
            [this](err const &error, std::size_t bytes_transferred) {});

        player2_->sendMsg(
            Packet::create(PacketType::CONN_PACKET,
                           ConnMsg::PLAYER2_INDICATION),
            [this](err const &error, std::size_t bytes_transferred) {});
        start();
    }

    void Game::start()
    {
        LOG_INF << "Game started between " << player1_->userName() << " and "
                << player2_->userName();
        LOG_INF << "p1.unique: " << player1_.unique()
                << " and p2.unique: " << player2_.unique();
        LOG_INF << "p1.use_count: " << player1_.use_count()
                << " , p2.use_count: " << player2_.use_count();
        readMove(PlayerIdentifer::X);
    }

    void Game::readMove(PlayerIdentifer identifer)
    {
        if (identifer == PlayerIdentifer::X)
        {
            player1_->readMove(
                [this](err const &error, std::size_t bytes_transferred) {
                    // LOG_INF << "Num of bytes recv p1: " << bytes_transferred;
                    // LOG_INF << "Read move error p1: " << error.message();
                    updateBoardAndCheckResult(PlayerIdentifer::X,
                                              player1_->getMove());
                });
        }
        else
        {
            player2_->readMove(
                [this](err const &error, std::size_t bytes_transferred) {
                    // LOG_INF << "Num of bytes recv p2: " << bytes_transferred;
                    // LOG_INF << "Read move error p2: " << error.message();
                    updateBoardAndCheckResult(PlayerIdentifer::O,
                                              player2_->getMove());
                });
        }
    }

    void Game::sendMove(PlayerIdentifer identifer, uint8_t move)
    {
        if (identifer == PlayerIdentifer::X)
        {
            player1_->sendMsg(
                Packet::create(PacketType::DATA_PACKET, move),
                [this](err const &error, std::size_t bytes_transferred) {
                    readMove(PlayerIdentifer::X);
                });
        }
        else
        {
            player2_->sendMsg(
                Packet::create(PacketType::DATA_PACKET, move),
                [this](err const &error, std::size_t bytes_transferred) {
                    readMove(PlayerIdentifer::O);
                });
        }
    }

    void Game::updateBoard(PlayerIdentifer id, uint8_t move)
    {
        // LOG_INF << "Game::updateBoard, move: " << int(move);
        if (!(move <= 9 and move >= 1))
        {
            LOG_ERR << "Cannot update the board.";
            return;
        }
        uint8_t rowNum, colNum;
        if (move % 3 != 0)
            rowNum = int(move / 3);
        else
            rowNum = int(move / 3) - 1;
        colNum                 = move - 3 * rowNum - 1;
        board_[rowNum][colNum] = id;
    }

    GameResult Game::checkResult()
    {
        if (moveCount_ == MAX_POSSIBLE_MOVES)
        {
            return GameResult::DRAW;
        }
        for (auto iter = 0; iter < 3; iter++)
        {
            if ((board_[iter][0] != Game::EMPTY) &&
                (board_[iter][0] == board_[iter][1]) &&
                (board_[iter][0] == board_[iter][2]))
            {
                return (board_[iter][0] == PlayerIdentifer::O)
                           ? GameResult::O_WIN
                           : GameResult::X_WIN;
            }

            if ((board_[0][iter] != Game::EMPTY) &&
                (board_[0][iter] == board_[1][iter]) &&
                (board_[0][iter] == board_[2][iter]))
            {
                return (board_[0][iter] == PlayerIdentifer::O)
                           ? GameResult::O_WIN
                           : GameResult::X_WIN;
            }
        }

        if ((board_[0][0] != Game::EMPTY) && (board_[0][0] == board_[1][1]) &&
            (board_[0][0] == board_[2][2]))
        {
            return (board_[0][0] == PlayerIdentifer::O) ? GameResult::O_WIN
                                                        : GameResult::X_WIN;
        }

        if ((board_[2][0] != Game::EMPTY) && (board_[2][0] == board_[1][1]) &&
            (board_[0][0] == board_[0][2]))
        {
            return (board_[0][0] == PlayerIdentifer::O) ? GameResult::O_WIN
                                                        : GameResult::X_WIN;
        }
        return GameResult::NO_RESULT;
    }

    void Game::updateBoardAndCheckResult(PlayerIdentifer id, uint8_t move)
    {
        // LOG_INF << "updateBoardAndCheckResult Read move: " << int(move);
        moveCount_++;
        updateBoard(id, move);
        auto result = checkResult();
        // auto result = NO_RESULT;
        switch (result)
        {
            case GameResult::NO_RESULT:
                if (id == PlayerIdentifer::X)
                {
                    sendMove(PlayerIdentifer::O, move);
                }
                else
                {
                    sendMove(PlayerIdentifer::X, move);
                }
                break;
            default:
                sendResultToPlayers(result);
        }
    }

    void Game::sendResultToPlayers(GameResult result)
    {
        player1_->sendMsg(
            Packet::create(PacketType::DATA_PACKET, result),
            [this](err const &error, std::size_t bytes_transferred) {
                player1_->socket().close();
            });
        player2_->sendMsg(
            Packet::create(PacketType::DATA_PACKET, result),
            [this](err const &error, std::size_t bytes_transferred) {
                player2_->socket().close();
            });
    }
} // namespace GameLib