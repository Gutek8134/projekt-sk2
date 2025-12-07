import socket
import pygame
from sys import argv
import threading
from queue import Queue
from drawing import draw_blank_board, draw_pieces, WHITE
from importlib import import_module
from board import Board, position, field, Color, Piece


def connect_to_server(client_socket: socket.socket, receiver_thread: threading.Thread):
    server_addr = argv[1] if len(argv) > 1 else "127.0.0.1"
    server_port = int(argv[2]) if len(argv) > 2 else 1337
    print(f"Connecting to server at {server_addr}:{server_port}")

    client_socket.connect((server_addr, server_port))
    print("Connected\nJoining game")

    receiver_thread.start()

    client_socket.send(b"join auto")


def end(client_socket: socket.socket):
    try:
        client_socket.send(b"leave")
        client_socket.shutdown(socket.SHUT_RDWR)
        client_socket.close()
    except OSError as e:
        print(e)


def receiver(client_socket: socket.socket, message_queue: Queue[str]):
    while True:
        message = client_socket.recv(4096).decode()
        message_queue.put(message)


def send_in_thread(client_socket: socket.socket, message: str):
    threading.Thread(target=lambda: client_socket.send(
        message.encode()), daemon=True).start()


def process_message(message: str):
    print(message)


def gui(message_queue: Queue[str]):
    board = Board()
    board.move(position(5, 5), position(5, 6))
    pygame.init()
    screen = pygame.display.set_mode((720, 720))
    pygame.display.set_caption("Gliński Chess")
    screen = pygame.display.set_mode((720, 720))
    screen.fill(WHITE)
    draw_blank_board(screen)

    clickable_moves_group = pygame.sprite.Group()
    sprites = pygame.sprite.Group(
        *draw_pieces(screen, board, clickable_moves_group))  # type: ignore

    # Game loop
    running: bool = True
    while running:
        while not message_queue.empty():
            process_message(message_queue.get())

        events = pygame.event.get()

        for event in events:
            if event.type == pygame.QUIT:
                running = False
                break

        screen.fill(WHITE)
        draw_blank_board(screen)

        sprites.update(events)
        clickable_moves_group.update(events)

        sprites.draw(screen)
        clickable_moves_group.draw(screen)
        pygame.display.flip()

    pygame.quit()


def main():
    client_socket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)

    message_queue: Queue[str] = Queue()
    receiver_thread = threading.Thread(target=receiver, args=(
        client_socket, message_queue), daemon=True)

    threading.Thread(target=connect_to_server, args=(
        client_socket, receiver_thread), daemon=True).start()

    gui(message_queue)

    end(client_socket)


if __name__ == "__main__":
    main()
