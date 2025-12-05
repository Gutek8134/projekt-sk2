#include "board.hpp"
#include "player_control.hpp"
#include "sockets.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <csignal>
#include <functional>
#include <memory>
#include <sstream>
#include <algorithm>

typedef struct pollfd pollfd;
int server_socket;
socklen_t sockaddr_in_size = sizeof(sockaddr_in);

std::vector<std::string> split(std::string string)
{
    std::istringstream iss(string);
    std::string part;
    std::vector<std::string> parts = {};
    while (std::getline(iss, part, ' '))
    {
        parts.push_back(part);
    }
    return parts;
}

bool enable_keepalive(int sock)
{
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) < 0)
    {
        perror("KEEP ALIVE");
        return false;
    }

    int idle = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) < 0)
    {
        perror("KEEP ALIVE");
        return false;
    }

    int interval = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) < 0)
    {
        perror("KEEP ALIVE");
        return false;
    }

    int maxpkt = 10;
    if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(maxpkt)) < 0)
    {
        perror("KEEP ALIVE");
        return false;
    }

    return true;
}

void handle_action(const int &player_id, const std::string &action)
{
    auto arguments = split(action);

    if (arguments.at(0) == "join")
    {
        player_control::add_player(player_id, ((arguments.at(1) == "auto") ? -1 : std::stoi(arguments.at(1))));
    }

    else if (arguments.at(0) == "move")
    {
        player_control::get_board(player_id)->move(arguments.at(1), arguments.at(2));
    }

    else if (arguments.at(0) == "leave")
    {
        player_control::remove_player(player_id);
    }
}

const bool send_messages(const int &player_id)
{
    if (!player_control::messages.contains(player_id))
    {
        std::cout << "Player " << player_id << " has not joined yet" << std::endl;
        return true;
    }

    while (!player_control::messages.at(player_id).empty())
    {
        const char *message = player_control::messages.at(player_id).front().c_str();
        int total_bytes_sent = 0, message_length = strlen(message);

        while (total_bytes_sent < message_length)
        {
            int bytes_sent = send(player_id, message, strlen(message), 0);

            if (bytes_sent > 0)
                total_bytes_sent += bytes_sent;

            if (bytes_sent == -1)
            {
                // Can't send anything more, move on
                if (errno == EWOULDBLOCK)
                    return true;

                // Something bad actually happened
                perror("SEND DATA");
                player_control::remove_player(player_id);
                return false;
            }
        }
        player_control::messages.at(player_id).pop();
    }
    return true;
}

void handle_interrupt(int)
{
    player_control::clear_players();
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
    exit(EXIT_SUCCESS);
}

void prepare_server(const uint16_t &port)
{
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        perror("SERVER SOCKET CREATE");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("SET REUSEADDR");
        exit(EXIT_FAILURE);
    }

    if (!set_nonblock(server_socket))
    {
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("BIND");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1)
    {
        perror("LISTEN");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
}

const bool handle_events(std::vector<pollfd> &poll_vector)
{
    std::vector<int> elements_to_remove = {};

    size_t initial_size = poll_vector.size();

    pollfd new_element;

    short server_events = poll_vector.at(0).revents;
    sockaddr_in client_address;
    int connection_fd;
    if (server_events & POLLIN)
    {
        while (true)
        {
            memset(&client_address, 0, sizeof(client_address));
            connection_fd = accept(server_socket, (sockaddr *)&client_address, &sockaddr_in_size);

            if (connection_fd < 0)
            {
                if (errno == EWOULDBLOCK)
                    break;

                perror("CONNECTION ERROR");
                return false;
            }
            else
            {
                std::cout << "Client connected id " << connection_fd << std::endl;
            }

            if (!set_nonblock(connection_fd))
            {
                return false;
            }
            if (!enable_keepalive(connection_fd))
            {
                return false;
            }

            new_element = pollfd();
            new_element.fd = connection_fd;
            new_element.events = POLLIN | POLLOUT | POLLHUP;
            new_element.revents = 0;

            poll_vector.push_back(new_element);
        }
    }

    for (size_t i = 1; i < initial_size; ++i)
    {
        // Nothing to do
        if (poll_vector.at(i).revents == 0)
            continue;
        bool skip = false;

        // Player disconnected
        if ((poll_vector.at(i).revents & (POLLHUP | POLLERR)))
        {
            int player_id = poll_vector.at(i).fd;
            std::shared_ptr<Board> game = player_control::get_board(player_id);

            int other_player_id;
            if (game->player_color(player_id) == Color::White)
            {
                other_player_id = game->get_player_id(Color::Black);
            }
            else if (game->player_color(player_id) == Color::Black)
            {
                other_player_id = game->get_player_id(Color::White);
            }
            else
            {
                std::cout << "Something is very wrong" << std::endl;
            }

            elements_to_remove.push_back(i);
            player_control::remove_player(player_id);
            if (other_player_id != -1)
            {
                player_control::messages.at(other_player_id).push("Opponent left");
                if (!game->has_game_ended())
                    player_control::messages.at(other_player_id).push("Win: Walkover");
            }

            skip = true;
        }

        // Incoming message from the client
        if ((poll_vector.at(i).revents & POLLIN))
        {
            int player_id = poll_vector.at(i).fd;
            std::string message = "";
            char message_part[4096];

            while (true)
            {
                memset(message_part, 0, sizeof(message_part));
                int read_bytes = recv(player_id, message_part, sizeof(message_part), 0);

                if (read_bytes == 0)
                    break;

                if (read_bytes == -1)
                {
                    if (errno != EWOULDBLOCK && !skip)
                    {
                        perror("RECEIVE");
                        player_control::remove_player(player_id);
                        elements_to_remove.push_back(i);
                        skip = true;
                    }

                    break;
                }

                message.append(message_part);
            }

            if (message.size() > 0)
                handle_action(player_id, message);
        }

        // Client ready to receive
        if ((poll_vector.at(i).revents & POLLOUT))
        {
            int player_id = poll_vector.at(i).fd;
            if (!send_messages(player_id) && !skip)
                elements_to_remove.push_back(i);
        }
    }

    for (size_t i = 0; i < elements_to_remove.size(); ++i)
    {
        std::cout << "Removed player form poll id " << poll_vector.at(elements_to_remove.at(i)).fd << std::endl;
        poll_vector.erase(poll_vector.begin() + elements_to_remove.at(i) - i);
    }

    return true;
}

int main(int argc, char *argv[])
{
    prepare_server(argc >= 2 ? atoi(argv[1]) : 1337);
    signal(SIGINT, handle_interrupt);

    std::vector<pollfd>
        poll_vector = {pollfd()};

    poll_vector.at(0).fd = server_socket;
    poll_vector.at(0).events = POLLIN;

    std::cout << "Server started" << std::endl;
    int number_of_events;
    while (true)
    {
        number_of_events = poll(&poll_vector.at(0), poll_vector.size(), 5000);

        if (number_of_events < 0)
        {
            perror("POLL FAIL");
            break;
        }

        if (number_of_events == 0)
        {
            continue;
        }

        if (!handle_events(poll_vector))
            break;
    }

    player_control::clear_players();
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);

    return 0;
}