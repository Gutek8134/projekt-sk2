#include "board.hpp"
#include "player_control.hpp"
#include "sockets.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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

void handle_action(const int &player_id, const std::string &action)
{
    auto arguments = split(action);

    if (arguments[0] == "join")
    {
        player_control::add_player(player_id, ((arguments[1] == "auto") ? -1 : std::stoi(arguments[1])));
    }

    else if (arguments[0] == "move")
    {
        player_control::get_board(player_id)->move(arguments[1], arguments[2]);
    }

    else if (arguments[0] == "leave")
    {
        player_control::remove_player(player_id);
    }
}

const bool send_messages(const int &player_id)
{
    while (!player_control::messages[player_id].empty())
    {
        const char *message = player_control::messages[player_id].front().c_str();
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
    }
    return true;
}

void handle_interrupt(int)
{
    player_control::clear_players();
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
}

void prepare_server(const uint16_t &port)
{
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0)
    {
        perror("SERVER SOCKET CREATE");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

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

            if (connection_fd < 0 && errno != EWOULDBLOCK)
            {
                perror("CONNECTION ERROR");
                return false;
            }
            else
            {
                std::cout << "Client connected" << std::endl;
            }

            if (!set_nonblock(connection_fd))
            {
                return false;
            }

            new_element = pollfd();
            new_element.fd = connection_fd;
            new_element.events = POLLIN | POLLOUT | POLLHUP;

            poll_vector.push_back(new_element);

            if (connection_fd == -1)
                break;
        }
    }

    for (size_t i = 0; i < initial_size; ++i)
    {
        // Nothing to do
        if (poll_vector.at(i).revents == 0)
            continue;
        bool skip = false;

        // Player disconnected
        if ((poll_vector.at(i).revents & POLLHUP) && !skip)
        {
            int player_id = poll_vector.at(i).fd;
            std::shared_ptr<Board> game = player_control::boards.at(player_control::games.at(player_id));

            player_control::messages[player_id].push("Opponent left");
            if (!game->has_game_ended())
                player_control::messages[player_id].push("Win: Walkover");
            game->player_left(game->player_color(player_id));
        }

        // Incoming message from the client
        if ((poll_vector.at(i).revents & POLLIN) && !skip)
        {
            int player_id = poll_vector.at(i).fd;
            std::string message = "";
            char message_part[4096];

            while (true)
            {
                memset(message_part, 0, sizeof(message_part));
                int read_bytes = recv(player_id, message_part, sizeof(message_part), 0);

                if (read_bytes == -1)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        perror("RECEIVE");
                        player_control::remove_player(player_id);
                        elements_to_remove.push_back(i);
                    }

                    break;
                }

                message.append(message_part);
            }

            handle_action(player_id, message);
        }

        // Client ready to receive
        if ((poll_vector.at(i).revents & POLLOUT) && !skip)
        {
            int player_id = poll_vector.at(i).fd;
            if (!send_messages(player_id))
                elements_to_remove.push_back(i);
        }
    }

    for (size_t i = 0; i < elements_to_remove.size(); ++i)
        poll_vector.erase(poll_vector.begin() + elements_to_remove.at(i) - i);

    return true;
}

int main(void)
{
    prepare_server(1337);
    signal(SIGINT, handle_interrupt);

    std::vector<pollfd>
        poll_vector = {pollfd()};

    poll_vector[0].fd = server_socket;
    poll_vector[0].events = POLLIN;

    int number_of_events;
    while (true)
    {
        std::cout << "Waiting for poll" << std::endl;
        number_of_events = poll(&poll_vector[0], poll_vector.size(), 5000);

        if (number_of_events < 0)
        {
            perror("POLL FAIL");
            break;
        }

        if (number_of_events == 0)
        {
            printf("poll timeout\n");
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