#include "board.hpp"
#include <iostream>

const bool move_is_orthogonal(short row_difference, char abs_column_difference);
const bool move_is_diagonal(short row_difference, char abs_column_difference);
short get_row(std::string position);
char get_column(std::string position);

const std::size_t cell_count = 91UL;

Board::Board()
{
    this->all_positions = {};
    this->white_pieces = {};
    this->black_pieces = {};
    this->board = {};
    board.reserve(cell_count);
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

const bool Board::move(std::string from, std::string to)
{
    // The game has finished
    if (white_won || black_won)
        return false;

    // Can't allow any illegal moves
    if (!move_is_legal(from, to))
        return false;

    Color player_color = board.at(from).color, capture_color = board.at(to).color;
    Piece player_piece = board.at(from).piece;

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

    // Check check
    _white_is_checked = position_under_attack(white_king_position, Color::Black);
    _black_is_checked = position_under_attack(black_king_position, Color::White);

    // Checkmate check
    if (player_color == Color::Black && _white_is_checked)
    {
        black_won = true;
        for (const auto &move : generate_king_moves(white_king_position))
            if (!position_under_attack(move, Color::Black))
            {
                black_won = false;
                break;
            }
    }
    if (player_color == Color::White && _black_is_checked)
    {
        white_won = true;
        for (const auto &move : generate_king_moves(black_king_position))
            if (!position_under_attack(move, Color::White))
            {
                white_won = false;
                break;
            }
    }

    return true;
}

void Board::show() const
{
    // TODO: make it look any better
    for (const auto &position : all_positions)
    {
        std::cout << board.at(position).to_string() << "\n";
    }
    std::flush(std::cout);
}

const bool Board::move_is_legal(std::string from, std::string to) const
{
    // Can't beat one's own piece
    Color color = board.at(from).color;
    if (color == board.at(to).color)
        return false;

    // Positions must exist in the board
    if (!all_positions.contains(from) || !all_positions.contains(to))
        return false;

    // Can't stay in place
    if (from == to)
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

    short row_difference = to_row - from_row;
    char abs_column_difference = abs(to_column - from_column);

    // Can't intentionally move into check
    if (player_color == Color::White && position_under_attack(to, Color::Black))
        return false;

    if (player_color == Color::Black && position_under_attack(to, Color::White))
        return false;

    return (row_difference == 1 && abs_column_difference <= 1) ||
           (row_difference == 0 && abs_column_difference <= 1) ||
           (row_difference == -1 && abs_column_difference <= 2) ||
           (row_difference == -2 && abs_column_difference == 1);
}

const bool Board::check_queen_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = to_row - from_row;
    char abs_column_difference = abs(to_column - from_column);

    return move_is_diagonal(row_difference, abs_column_difference) ||
           move_is_orthogonal(row_difference, abs_column_difference);
}

const bool Board::check_rook_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = to_row - from_row;
    char abs_column_difference = abs(to_column - from_column);

    return move_is_orthogonal(row_difference, abs_column_difference);
}

const bool Board::check_bishop_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = to_row - from_row;
    char abs_column_difference = abs(to_column - from_column);

    return move_is_diagonal(row_difference, abs_column_difference);
}

const bool Board::check_knight_move(std::string from, std::string to) const
{
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    short row_difference = to_row - from_row;
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
    // TODO: initial two step move
    // TODO: promotions
    // optional TODO: en passant
    short from_row = get_row(from), to_row = get_row(to);
    char from_column = get_column(from), to_column = get_column(to);

    if (player_color == Color::White)
    {
        return abs(to_column - from_column) == 1                                   // Move in row, able to capture
               || (to_row - from_row == 1 && board.at(to).piece == Piece::NoPiece) // Move in column, unable to capture
            ;
    }
    return abs(to_column - from_column) == -1                                  // Move in row, able to capture
           || (to_row - from_row == 1 && board.at(to).piece == Piece::NoPiece) // Move in column, unable to capture
        ;
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

    return abs_column_difference == 2 * (-row_difference) || // left-right
           2 * abs_column_difference == (-row_difference);   // down diagonally
}

short get_row(std::string position)
{
    return static_cast<short>(std::stoi(position.substr(1)));
}

char get_column(std::string position)
{
    return position[0];
}