#pragma once

#include "board.hpp"
#include <memory>
#include <poll.h>
#include <queue>

namespace player_control
{
    /// player id -> game id
    extern std::unordered_map<int, int> games;
    /// messages to send
    extern std::unordered_map<int, std::queue<std::string>> messages;
    /// game id -> game board
    extern std::unordered_map<int, std::shared_ptr<Board>> boards;

    void clear_players();
    void add_player(const int &player_id, int game_id = -1, Color preferred_color = Color::NoColor);
    void remove_player(const int &player_id);

    std::shared_ptr<Board> get_board(const int &player_id);
}