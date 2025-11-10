#include "board.hpp"
#include <iostream>

int main(void)
{
    Board board = Board(), copy;
    copy = board;
    board.load_board(board.serialize());

    std::cout << (copy.state_equal(board) ? "True" : "False") << std::endl;
    return 0;
}