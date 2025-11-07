#pragma once
#include <string>
#include <stdexcept>

enum Piece : unsigned short
{
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn,
    NoPiece
};

enum Color : unsigned short
{
    White,
    Black,
    NoColor
};

std::string piece_to_string(Piece piece);

std::string color_to_string(Color color);