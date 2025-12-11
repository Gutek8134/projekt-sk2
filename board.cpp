#include "board.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

const bool move_is_orthogonal(short row_difference, char abs_column_difference);
const bool move_is_diagonal(short row_difference, char abs_column_difference);
short get_row(std::string position);
char get_column(std::string position);
bool position_on_end(std::string position, Color color);
bool position_on_pawn_initial_row(short row, char col, Color pawn_color);
short get_relative_row_difference(short from_row, short to_row, char from_col, char to_col);

const std::size_t cell_count = 91UL;

Board::Board(int _game_id, int _black_player_id, int _white_player_id, bool _cheat_board) : black_player_id(_black_player_id), white_player_id(_white_player_id), game_id(_game_id), cheat_board(_cheat_board)
{
    this->all_positions = {};
    this->white_pieces = {};
    this->black_pieces = {};
    this->board = {};
    this->active_color = Color::Black;
    board.reserve(cell_count);
    white_won = false;
    black_won = false;
    _white_is_checked = false;
    _black_is_checked = false;

    std::string row, column, position;
    unsigned short row_length, row_distance_from_f;
    for (unsigned short row_index = 1; row_index <= 11; ++row_index)
    {
        row = std::to_string(row_index);
        row_length = line_length.at(row);
        row_distance_from_f = (row_length - 1) / 2;

        for (char column_index = 'f' - row_distance_from_f; column_index <= 'f' + row_distance_from_f; ++column_index)
        {
            column = column_index;
            position = column + row;
            this->all_positions.insert(position);

            field empty_field =
                {
                    position,
                    Piece::NoPiece,
                    Color::NoColor,
                };

            board[position] = empty_field;
        }
    }

#pragma region fill_board
    std::string symmetrical_position;
    for (const auto &position : king_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::King;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::King;

        white_king_position = position;
        white_pieces.insert(position);
        black_king_position = symmetrical_position;
        black_pieces.insert(symmetrical_position);
    }

    for (const auto &position : queen_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Queen;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Queen;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }

    for (const auto &position : rook_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Rook;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Rook;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : bishop_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Bishop;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Bishop;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : knight_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Knight;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Knight;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : pawn_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Pawn;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Pawn;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }

#pragma endregion fill_board
}

void Board::reset()
{
    this->all_positions = {};
    this->white_pieces = {};
    this->black_pieces = {};
    this->board = {};
    this->active_color = Color::Black;
    board.reserve(cell_count);
    white_won = false;
    black_won = false;
    _white_is_checked = false;
    _black_is_checked = false;

    std::string row, column, position;
    unsigned short row_length, row_distance_from_f;
    for (unsigned short row_index = 1; row_index <= 11; ++row_index)
    {
        row = std::to_string(row_index);
        row_length = line_length.at(row);
        row_distance_from_f = (row_length - 1) / 2;

        for (char column_index = 'f' - row_distance_from_f; column_index <= 'f' + row_distance_from_f; ++column_index)
        {
            column = column_index;
            position = column + row;
            this->all_positions.insert(position);

            field empty_field =
                {
                    position,
                    Piece::NoPiece,
                    Color::NoColor,
                };

            board[position] = empty_field;
        }
    }

#pragma region fill_board
    std::string symmetrical_position;
    for (const auto &position : king_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::King;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::King;

        white_king_position = position;
        white_pieces.insert(position);
        black_king_position = symmetrical_position;
        black_pieces.insert(symmetrical_position);
    }

    for (const auto &position : queen_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Queen;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Queen;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }

    for (const auto &position : rook_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Rook;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Rook;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : bishop_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Bishop;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Bishop;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : knight_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Knight;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Knight;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }
    for (const auto &position : pawn_positions)
    {
        board[position].color = Color::White;
        board[position].piece = Piece::Pawn;

        symmetrical_position = Board::get_symmetrical_position(position);
        board[symmetrical_position].color = Color::Black;
        board[symmetrical_position].piece = Piece::Pawn;

        white_pieces.insert(position);
        black_pieces.insert(symmetrical_position);
    }

#pragma endregion fill_board
}

const bool Board::move(std::string from, std::string to, Color player_color)
{
    // Awaiting second player
    // If both ids are -1, continues assuming testing purposes
    if ((black_player_id == -1) ^ (white_player_id == -1))
        return false;

    if (board.at(from).color != player_color)
        return false;

    Color capture_color = board.at(to).color;
    // It's the other player's move
    if (player_color != active_color || ((black_player_id == -1) && (white_player_id == -1)))
        return false;

    // The game has finished
    if (white_won || black_won)
        return false;

    // Can't allow any illegal moves
    if (!move_is_legal(from, to))
        return false;

    Piece player_piece = board.at(from).piece;

    // Win check
    if (board.at(to).piece == Piece::King)
    {
        if (player_color == Color::White)
            white_won = true;
        else
            black_won = true;
    }

    // Move
    board.at(to).color = player_color;
    board.at(to).piece = player_piece;

    board.at(from).color = Color::NoColor;
    board.at(from).piece = Piece::NoPiece;

    if (player_color == Color::White)
    {
        white_pieces.erase(from);
        white_pieces.insert(to);
        if (player_piece == Piece::King)
            white_king_position = to;
    }

    if (player_color == Color::Black)
    {
        black_pieces.erase(from);
        black_pieces.insert(to);
        if (player_piece == Piece::King)
            black_king_position = to;
    }

    // Capture
    if (capture_color == Color::White)
    {
        white_pieces.erase(to);
    }
    else if (capture_color == Color::Black)
    {
        black_pieces.erase(to);
    }

    // Switch active player
    active_color = (active_color == Color::Black) ? Color::White : Color::Black;

    // // Check check
    // _white_is_checked = position_under_attack(white_king_position, Color::Black);
    // _black_is_checked = position_under_attack(black_king_position, Color::White);

    // // Checkmate check
    // if (player_color == Color::Black && _white_is_checked)
    // {
    //     black_won = true;
    //     for (const auto &move : generate_king_moves(white_king_position))
    //         if (!position_under_attack(move, Color::Black))
    //         {
    //             black_won = false;
    //             break;
    //         }
    // }
    // if (player_color == Color::White && _black_is_checked)
    // {
    //     white_won = true;
    //     for (const auto &move : generate_king_moves(black_king_position))
    //         if (!position_under_attack(move, Color::White))
    //         {
    //             white_won = false;
    //             break;
    //         }
    // }

    return true;
}

const std::string Board::serialize() const
{
    std::string serialized_board = "";
    for (const auto &position : all_positions)
    {
        serialized_board += board.at(position).to_string() + "\n";
    }
    return serialized_board;
}

void Board::load_board(std::string serialized_board)
{
    std::stringstream board_stream = std::stringstream(serialized_board);

    std::string line;

    while (getline(board_stream, line, '\n'))
    {
        if (line[0] == 'E')
        {
            board[line.substr(2)].color = Color::NoColor;
            board[line.substr(2)].piece = Piece::NoPiece;
            continue;
        }

        std::string position;
        if (line[0] == 'W')
        {
            position = line.substr(3);
            board[position].color = Color::White;
        }
        else if (line[0] == 'B')
        {
            position = line.substr(3);
            board[position].color = Color::Black;
        }
        switch (line[1])
        {
        case 'K':
            board[position].piece = Piece::King;
            break;
        case 'Q':
            board[position].piece = Piece::Queen;
            break;
        case 'B':
            board[position].piece = Piece::Bishop;
            break;
        case 'R':
            board[position].piece = Piece::Rook;
            break;
        case 'N':
            board[position].piece = Piece::Knight;
            break;
        case 'P':
            board[position].piece = Piece::Pawn;
            break;

        default:
            break;
        }
    }
}

void Board::show() const
{
    // TODO: make it look any better
    std::cout << serialize();
    std::flush(std::cout);
}

const bool Board::move_is_legal(std::string from, std::string to) const
{
    // Positions must exist in the board
    if (!all_positions.contains(from) || !all_positions.contains(to))
        return false;

    // Can't stay in place
    if (from == to)
        return false;

    // Can't beat one's own piece
    Color color = board.at(from).color;
    if (color == board.at(to).color)
        return false;

    Piece piece = board.at(from).piece;
    switch (piece)
    {
    case Piece::King:
        return check_king_move(from, to, color);

    case Piece::Queen:
        return check_queen_move(from, to);

    case Piece::Rook:
        return check_rook_move(from, to);

    case Piece::Bishop:
        return check_bishop_move(from, to);

    case Piece::Knight:
        return check_knight_move(from, to);

    case Piece::Pawn:
        return check_pawn_move(from, to, color);

    default:
        return false;
    }
}

const bool Board::check_king_move(std::string from, std::string to, Color player_color) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    // Let's ignore this one for simplicity
    // Can't intentionally move into check
    // //! possible infinite recursion, though it passed a simple test
    // if (player_color == Color::White && position_under_attack(to, Color::Black))
    //     return false;

    // if (player_color == Color::Black && position_under_attack(to, Color::White))
    //     return false;

    return (row_difference == 1 && abs_column_difference <= 1) ||
           (row_difference == 0 && abs_column_difference <= 1) ||
           (row_difference == -1 && abs_column_difference <= 2) ||
           (row_difference == -2 && abs_column_difference == 1);
}

const bool Board::check_queen_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    return move_is_diagonal(row_difference, abs_column_difference) ||
           move_is_orthogonal(row_difference, abs_column_difference);
}

const bool Board::check_rook_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    return move_is_orthogonal(row_difference, abs_column_difference);
}

const bool Board::check_bishop_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    return move_is_diagonal(row_difference, abs_column_difference);
}

const bool Board::check_knight_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    return (row_difference == 2 && abs_column_difference == 1) ||
           (row_difference == 1 && abs_column_difference == 2) ||
           (row_difference == -1 && abs_column_difference == 3) ||
           (row_difference == -2 && abs_column_difference == 3) ||
           (row_difference == -3 && abs_column_difference == 1) ||
           (row_difference == -3 && abs_column_difference == 2);
}

const bool Board::check_pawn_move(std::string from, std::string to, Color player_color) const
{
    // optional TODO: en passant
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = get_relative_row_difference(from_row, to_row, from_column, to_column);
    char abs_column_difference = abs(to_column - from_column);

    if (player_color == Color::White)
    {
        return (abs_column_difference == 1 && row_difference == 0) || // Move in row, able to capture
               (row_difference == 1 && abs_column_difference == 0 &&
                board.at(to).piece == Piece::NoPiece) || // Move in column, unable to capture
               (position_on_pawn_initial_row(from_row, from_column, player_color) &&
                abs_column_difference == 0 && row_difference == 2 &&
                board.at(to).piece == Piece::NoPiece) // Two-step move from the initial row
            ;
    }

    return (abs_column_difference == 1 && row_difference == -1) || // Move in row, able to capture
           (row_difference == -1 && abs_column_difference == 0 &&
            board.at(to).piece == Piece::NoPiece) || // Move in column, unable to capture
           (position_on_pawn_initial_row(from_row, from_column, player_color) &&
            abs_column_difference == 0 && row_difference == -2 &&
            board.at(to).piece == Piece::NoPiece) // Two-step move from the initial row
        ;
}

const bool Board::promote(std::string position, Piece to_piece)
{
    field pawn_field = board.at(position);

    if (pawn_field.piece != Piece::Pawn)
        return false;

    if (!position_on_end(position, pawn_field.color))
        return false;

    board.at(position).piece = to_piece;

    return true;
}

const bool Board::position_under_attack(std::string checked_position, Color attacker) const
{
    if (attacker == Color::White)
        for (const auto &white_position : white_pieces)
        {
            if (move_is_legal(white_position, checked_position))
                return true;
        }
    else
        for (const auto &white_position : black_pieces)
        {
            if (move_is_legal(white_position, checked_position))
                return true;
        }
    return false;
}

const field &Board::get_field(std::string at) const
{
    return board.at(at);
}

const std::unordered_set<std::string> &Board::get_all_positions() const
{
    return all_positions;
}

const std::string Board::get_symmetrical_position(std::string position)
{
    short row = get_row(position), new_row;
    std::string column = std::string(1, get_column(position));

    // std::cout << "Position: " << position << "\n"
    //           << "Col: " << column << " Row:" << row << std::endl;

    new_row = line_length.at(column) - row + 1;

    // std::cout << new_row << std::endl;

    return std::format("{}{}", column, new_row);
}

const std::vector<std::string> Board::generate_king_moves(std::string from_position) const
{
    short row = get_row(from_position);
    char col = get_column(from_position);

    // Generate all king's moves
    std::vector<std::pair<short, char>> move_parts =
        {
            {row + 1, col - 1},
            {row + 1, col},
            {row + 1, col + 1},
            {row, col - 1},
            {row, col + 1},
            {row - 1, col - 2},
            {row - 1, col - 1},
            {row - 1, col},
            {row - 1, col + 1},
            {row - 1, col + 2},
            {row - 2, col - 1},
            {row - 2, col + 1},
        };

    std::vector<std::string> moves = {};

    for (const auto &part : move_parts)
        moves.push_back(std::to_string(part.first) + part.second);

    // Remove those that go outside the board
    for (size_t i = 0; i < moves.size(); ++i)
        if (!all_positions.contains(moves.at(i)))
            moves.erase(moves.begin() + i);

    return moves;
}

const bool move_is_orthogonal(short row_difference, char abs_column_difference)
{
    return abs_column_difference == 0 ||             // move in column
           row_difference == 0 ||                    // move in row
           abs_column_difference == -row_difference; // move in row flipped on X axis
}

const bool move_is_diagonal(short row_difference, char abs_column_difference)
{
    if (row_difference > 0)
        return row_difference == abs_column_difference; // up diagonally

    return abs_column_difference == 2 * row_difference || // left-right
           2 * abs_column_difference == row_difference;   // down diagonally
}

const bool Board::player_joined(int player_id, Color player_color)
{
    if (player_color == Color::NoColor)
        throw std::invalid_argument("Invalid color in player joined: No Color");

    if (player_color == Color::Black)
    {
        if (black_player_id != -1)
            return false;
        black_player_id = player_id;
    }
    else
    {
        if (white_player_id != -1)
            return false;
        white_player_id = player_id;
    }
    return true;
}

void Board::player_left(Color player_color)
{
    if (player_color == Color::NoColor)
        throw std::invalid_argument("Invalid color in player left: No Color");

    if (player_color == Color::Black)
        black_player_id = -1;
    else
        white_player_id = -1;

    if (!has_game_ended())
    {
        if (player_color == Color::Black)
            white_won = true;
        else
            black_won = true;
    }
}

const int Board::get_player_id(Color player_color) const
{
    if (player_color == Color::NoColor)
        throw std::invalid_argument("Invalid color in player joined: No Color");

    if (player_color == Color::Black)
        return black_player_id;

    if (player_color == Color::White)
        return white_player_id;

    throw std::invalid_argument("Invalid color in player joined: Not a Color");
}

short get_row(std::string position)
{
    return static_cast<short>(std::stoi(position.substr(1)));
}

char get_column(std::string position)
{
    return position[0];
}

bool position_on_end(std::string position, Color color)
{
    short row = get_row(position);
    std::string col = std::to_string(get_column(position));

    return (row == 1 && color == Color::Black) ||
           (row == line_length.at(col) && color == Color::White);
}

bool position_on_pawn_initial_row(short row, char col, Color pawn_color)
{
    return (pawn_color == Color::White && row == 5 - abs('f' - col)) ||
           (pawn_color == Color::Black && row == 7);
}

// Normalizes into f-column relative movement
short get_relative_row_difference(short from_row, short to_row, char from_column, char to_column)
{
    // d4 g6
    // normal row difference 2
    // expected 0
    // abs... = 2
    // got 0

    // h6 e5
    // normal row difference -1
    // expected -3
    // abs... = 2
    // got -3

    short row_difference = to_row - from_row;

    // Relative to itself means no changes
    if (from_column == 'f')
        return row_difference;

    // Decrease difference by 1 for each row moved against "board current"
    if (from_column > 'f' && to_column < from_column)
        row_difference -= abs(std::max(to_column, 'f') - from_column);

    else if (from_column < 'f' && to_column > from_column)
        row_difference -= abs(std::min(to_column, 'f') - from_column);

    return row_difference;
}