#include "player_control.hpp"
#include <arpa/inet.h>
#include <climits>
#include <iostream>

std::unordered_set<int> free_gids = {};

namespace player_control
{
    std::unordered_map<int, int> games = {};
    std::unordered_map<int, std::shared_ptr<Board>> boards = {};
    std::unordered_map<int, std::queue<std::string>> messages = {};

    void clear_players()
    {
        boards.clear();
        for (const auto &pidgid : games)
        {
            int player_id = pidgid.first;
            shutdown(player_id, SHUT_RDWR);
            close(player_id);
        }
        games.clear();
    }

    void add_player(const int &player_id, int game_id, Color preferred_color)
    {
        bool cheats = game_id == 42069;

        if (game_id == -1)
        {
            if (!free_gids.empty())
            {
                auto it = free_gids.begin();
                game_id = *it;
                free_gids.erase(it);
            }
            else
            {
                // First board without both players or biggest game id +1
                int next_game_id = 0;
                for (const auto &gid_board : boards)
                {

                    if ((preferred_color == Color::NoColor && !gid_board.second->has_both_players()) ||
                        (preferred_color == Color::Black && !gid_board.second->has_black_player()) ||
                        (preferred_color == Color::White && !gid_board.second->has_white_player()))
                    {
                        next_game_id = gid_board.first;
                        break;
                    }

                    if (gid_board.first >= next_game_id)
                        next_game_id = gid_board.first + 1;
                }

                game_id = next_game_id;
            }
        }

        games[player_id] = game_id;
        if (!boards.contains(game_id))
        {
            Board *board = new Board(game_id,
                                     ((preferred_color == Color::Black) ? player_id : -1),
                                     ((preferred_color == Color::White || preferred_color == Color::NoColor) ? player_id : -1),
                                     cheats);
            boards.insert({game_id, std::shared_ptr<Board>(board)});
        }
        else
        {
            auto board = boards.at(game_id);

            if (board->has_white_player())
                messages.at(board->get_player_id(Color::White)).push("Black player joined");
            else if (board->has_black_player())
                messages.at(board->get_player_id(Color::Black)).push("White player joined");

            if (preferred_color == Color::NoColor)
                board->player_joined(player_id);
            else
                board->player_joined(player_id, preferred_color);
        }

        cheats = boards.at(game_id)->cheat_board || cheats;

        messages[player_id] = std::queue<std::string>();
        messages.at(player_id).push(std::format("Connected to game {}\nPlayer id: {}\nColor: {}", game_id, player_id, color_to_string(boards.at(game_id)->player_color(player_id))));
        if (cheats)
            messages.at(player_id).push(std::format("load\n{}", boards.at(game_id)->serialize()));

        std::cout << "Player " << player_id << " joined" << std::endl;
        if (!boards.at(game_id)->has_both_players())
        {
            messages.at(player_id).push("Waiting for other player");
        }
        else
        {
            boards.at(game_id)->reset();
            messages.at(boards.at(game_id)->get_player_id(Color::White)).push("Game started");
            messages.at(boards.at(game_id)->get_player_id(Color::Black)).push("Game started");
            std::cout << "Game " << game_id << " started" << std::endl;
        }
    }

    void remove_player(const int &player_id)
    {
        if (!messages.contains(player_id))
            return;

        const bool both_players_present = get_board(player_id)->has_both_players();

        std::cout << "Player left id " << player_id << "\nBoard white id " << get_board(player_id)->get_player_id(Color::White) << " black id " << get_board(player_id)->get_player_id(Color::Black) << std::endl;
        get_board(player_id)->player_left(player_id);

        if (both_players_present)
        {
            if (get_board(player_id)->player_color(player_id) == Color::White)
            {
                messages.at(get_board(player_id)->get_player_id(Color::Black)).push("Opponent left");
                if (!get_board(player_id)->has_game_ended())
                    messages.at(get_board(player_id)->get_player_id(Color::Black)).push("Win: walkover");
            }
            else if (get_board(player_id)->player_color(player_id) == Color::Black)
            {
                messages.at(get_board(player_id)->get_player_id(Color::Black)).push("Opponent left");
                if (!get_board(player_id)->has_game_ended())
                    messages.at(get_board(player_id)->get_player_id(Color::Black)).push("Win: walkover");
            }
        }
        // Last player left
        else
            boards.erase(games.at(player_id));

        games.erase(player_id);
        messages.erase(player_id);

        shutdown(player_id, SHUT_RDWR);
        close(player_id);
    }

    std::shared_ptr<Board> get_board(const int &player_id)
    {
        return boards.at(games.at(player_id));
    }
}