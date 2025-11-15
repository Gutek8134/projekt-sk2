#pragma once
#include "pieces.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <format>

typedef struct field
{
    std::string position;
    Piece piece;
    Color color;

    const std::string to_string() const
    {
        if (piece == Piece::NoPiece)
            return std::format("E {}", position);

        return std::format("{}{} {}", color_to_string(color), piece_to_string(piece), position);
    }

    const bool equal(const struct field &other) const
    {
        return position == other.position && piece == other.piece && color == other.color;
    }
} field;

/// @brief Board for Hexagonal Chess
class Board
{

protected:
    std::unordered_map<std::string, field> board;
    std::unordered_set<std::string> all_positions, white_pieces, black_pieces;
    std::string white_king_position, black_king_position;
    bool _white_is_checked, _black_is_checked;
    bool black_won, white_won;

public:
    Board();
    const field &get_field(std::string at) const;
    const std::unordered_set<std::string> &get_all_positions() const;
    static const std::string get_symmetrical_position(std::string position);
    const bool move(std::string from, std::string to);
    const bool promote(std::string position, Piece to);
    const bool white_is_checked() const { return _white_is_checked; }
    const bool black_is_checked() const { return _black_is_checked; }
    const bool has_white_won() const { return white_won; }
    const bool has_black_won() const { return black_won; }
    const std::string serialize() const;
    void load_board(std::string serialized_board);

    void show() const;

    const bool state_equal(Board other) const
    {
        for (const auto &position : all_positions)
            if (!board.at(position).equal(other.get_field(position)))
                return false;
        return true;
    }

private:
    const bool move_is_legal(std::string from, std::string to) const;
    const bool check_king_move(std::string from, std::string to, Color player_color) const;
    const bool check_queen_move(std::string from, std::string to) const;
    const bool check_rook_move(std::string from, std::string to) const;
    const bool check_bishop_move(std::string from, std::string to) const;
    const bool check_knight_move(std::string from, std::string to) const;
    const bool check_pawn_move(std::string from, std::string to, Color color) const;
    const bool position_under_attack(std::string position, Color attacker) const;
    const std::vector<std::string> generate_king_moves(std::string from_position) const;
};

const std::vector<std::string>
    king_positions = {"g1"},
    queen_positions = {"e1"},
    rook_positions = {"c1", "i1"},
    bishop_positions = {"f1", "f2", "f3"},
    knight_positions = {"d1", "h1"},
    pawn_positions = {"b1", "c2", "d3", "e4", "f5", "g4", "h3", "i2", "j1"};

const std::unordered_map<std::string, unsigned short> line_length =
    {
        {"a", 6},
        {"b", 7},
        {"c", 8},
        {"d", 9},
        {"e", 10},
        {"f", 11},
        {"g", 10},
        {"h", 9},
        {"i", 8},
        {"j", 7},
        {"k", 6},
        {"1", 11},
        {"2", 11},
        {"3", 11},
        {"4", 11},
        {"5", 11},
        {"6", 11},
        {"7", 9},
        {"8", 7},
        {"9", 5},
        {"10", 3},
        {"11", 1}};

const std::string columns = "abcdefghijk";

short get_row(std::string position);

char get_column(std::string position);