#include "sockets.hpp"
#include <fcntl.h>
#include <stdio.h>

const int on = 1;
const int off = 0;

bool set_nonblock(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1)
    {
        perror("NONBLOCK GET FLAGS");
        return false;
    }

    if (fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("NONBLOCK SET FLAGS");
        return false;
    }
    return true;
}