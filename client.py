import socket
import pygame
from sys import argv
import threading
from queue import Queue
from drawing import draw_blank_board, draw_pieces, WHITE
from importlib import import_module
from board import Board, position, field, Color, Piece


def end(client_socket: socket.socket):
    try:
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
    pass


def gui(message_queue: Queue[str], receiver_thread: threading.Thread):
    board = Board()
    pygame.init()
    screen = pygame.display.set_mode((720, 720))
    pygame.display.set_caption("Hello Pygame")
    screen = pygame.display.set_mode((720, 720))
    screen.fill(WHITE)
    draw_blank_board(screen)

    sprites = pygame.sprite.Group(*draw_pieces(screen, board))

    # Game loop
    running: bool = True
    while running:
        while not message_queue.empty():
            process_message(message_queue.get())

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                break
        sprites.draw(screen)
        pygame.display.flip()

    pygame.quit()


def main():
    client_socket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)

    message_queue: Queue[str] = Queue()
    receiver_thread = threading.Thread(target=receiver, args=(
        client_socket, message_queue), daemon=True)

    gui(message_queue, receiver_thread)

    end(client_socket)


if __name__ == "__main__":
    main()
