#include "pieces.hpp"

std::string piece_to_string(Piece piece)
{
    switch (piece)
    {
    case Piece::King:
        return "K";
    case Piece::Queen:
        return "Q";
    case Piece::Rook:
        return "R";
    case Piece::Bishop:
        return "B";
    case Piece::Knight:
        return "N";
    case Piece::Pawn:
        return "P";
    case Piece::NoPiece:
        return " ";

    default:
        throw std::out_of_range("Invalid piece.");
        break;
    }
}

std::string color_to_string(Color color)
{
    switch (color)
    {
    case Color::White:
        return "W";
    case Color::Black:
        return "B";
    case Color::NoColor:
        return " ";

    default:
        throw std::out_of_range("Invalid color.");
        break;
    }
}