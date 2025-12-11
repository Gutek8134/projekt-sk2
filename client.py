import socket
import pygame
import pygame.freetype
from sys import argv
import threading
from queue import Queue
from drawing import draw_blank_board, draw_pieces, WHITE, PieceSprite
from importlib import import_module
from board import Board, Color, position, columns


def connect_to_server(client_socket: socket.socket, receiver_thread: threading.Thread) -> None:
    server_addr = argv[1] if len(argv) > 1 else "127.0.0.1"
    server_port = int(argv[2]) if len(argv) > 2 else 1337
    print(f"Connecting to server at {server_addr}:{server_port}")

    try:
        client_socket.connect((server_addr, server_port))
    except ConnectionRefusedError:
        print("Connection refused")
        return
    print("Connected\nJoining game")

    receiver_thread.start()

    client_socket.send(b"join 42069")


def end(client_socket: socket.socket) -> None:
    try:
        client_socket.send(b"leave")
        client_socket.shutdown(socket.SHUT_RDWR)
        client_socket.close()
    except OSError as e:
        print(e)


def receiver(client_socket: socket.socket, message_queue: Queue[str]) -> None:
    while True:
        message = client_socket.recv(4096).decode()
        message_queue.put(message)


def _send_in_thread(client_socket: socket.socket, message: str, lock: threading.Lock) -> None:
    with lock:
        client_socket.send(message.encode())


def send_in_thread(client_socket: socket.socket, message: str, lock: threading.Lock) -> None:
    threading.Thread(target=_send_in_thread, args=(
        client_socket, message, lock), daemon=True).start()


def process_message(message: str, board: Board, sprites: pygame.sprite.Group, screen: pygame.Surface, clickable_moves_group: pygame.sprite.Group):
    if len(message) == 0:
        return
    if message == "blocked" or message.startswith("error"):
        board.retract_last_move()
        board.awaiting_approval = False

    elif message == "accepted":
        board.awaiting_approval = False

    elif message.endswith("Game started"):
        board.game_on = True

    multipart_message = message.split()

    if len(multipart_message) > 1:
        if multipart_message[0] == "move" and len(multipart_message) == 3:
            board.apply_move(position(columns.index(multipart_message[1][0]), int(
                multipart_message[1][1:])), position(columns.index(multipart_message[2][0]), int(multipart_message[2][1:])))

    multipart_message = message.splitlines()
    if len(multipart_message) > 0:
        # print(f'"{multipart_message[0]}"')
        if multipart_message[0] == "load":
            board.load(
                "\n".join(part for part in multipart_message[1:] if len(part.split()) == 2))
            sprites.empty()
            sprites.add(draw_pieces(screen, board, clickable_moves_group))
            for sprite in sprites:
                sprite: PieceSprite
                sprite.group = sprites

        for line in multipart_message:
            if line.startswith("Color"):
                board.player_color = Color.Black if line[-1] == "B" else Color.White

    if "Win" in message:
        board.game_on = False

    print(message)


def gui(message_queue: Queue[str], client_socket: socket.socket) -> None:
    board = Board(client_socket)
    pygame.init()
    screen = pygame.display.set_mode((720, 720))
    pygame.display.set_caption("Gliński Chess")
    screen = pygame.display.set_mode((720, 720))
    screen.fill(WHITE)
    draw_blank_board(screen)

    font = pygame.freetype.SysFont("DejaVu Sans", 24)

    clickable_moves_group = pygame.sprite.Group()
    sprites = pygame.sprite.Group(
        *draw_pieces(screen, board, clickable_moves_group))  # type: ignore

    for sprite in sprites:
        sprite: PieceSprite
        sprite.group = sprites

    # Game loop
    running: bool = True
    while running:
        while not message_queue.empty():
            process_message(message_queue.get(), board, sprites,
                            screen, clickable_moves_group)

        events = pygame.event.get()

        for event in events:
            if event.type == pygame.QUIT:
                running = False
                break

        screen.fill(WHITE)
        draw_blank_board(screen)

        sprites.update(events)
        if not board.awaiting_approval:
            clickable_moves_group.update(events)

        sprites.draw(screen)
        if not board.awaiting_approval:
            clickable_moves_group.draw(screen)

        font.render_to(
            screen, (10, 20), "Player color:")
        font.render_to(
            screen, (10, 45), board.player_color.name if board.player_color is not Color.NoColor else 'Not connected')

        font.render_to(screen, (600, 20), "Wait" if board.player_color !=
                       board.current_turn else "Your turn")
        pygame.display.flip()

    pygame.quit()


def main() -> None:
    client_socket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)

    message_queue: Queue[str] = Queue()
    receiver_thread = threading.Thread(target=receiver, args=(
        client_socket, message_queue), daemon=True)

    threading.Thread(target=connect_to_server, args=(
        client_socket, receiver_thread), daemon=True).start()

    gui(message_queue, client_socket)

    end(client_socket)


if __name__ == "__main__":
    main()
