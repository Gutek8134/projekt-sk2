import socket
from sys import argv
import threading
from functools import partial
import atexit


def end(client_socket: socket.socket):
    client_socket.shutdown(socket.SHUT_RDWR)
    client_socket.close()


def receiver(client_socket: socket.socket):
    pass


def main():
    client_socket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
    client_socket.connect(
        ("127.0.0.1", int(argv[1]) if len(argv) >= 2 else 1337))
    threading.Thread(target=receiver, args=(client_socket,), daemon=True)
    client_socket.send(b"join auto")
    print("sent")
    message = client_socket.recv(4096).decode()
    print(message)
    client_socket.shutdown(socket.SHUT_RDWR)
    client_socket.close()


if __name__ == "__main__":
    main()
