import pygame
from math import sin, cos, pi
from board import Board, position

WHITE = 255, 255, 255


class PieceSprite(pygame.sprite.Sprite):
    def __init__(self, image: pygame.Surface, x: int, y: int):
        super().__init__()
        self.image = image
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.visible = True

    def update(self, events: list[pygame.event.Event]):
        for event in events:
            if event.type == pygame.MOUSEBUTTONUP:
                print(event.pos)
                if self.rect.collidepoint(event.pos):
                    pass


def draw_blank_board(screen: pygame.Surface) -> None:
    image = pygame.image.load("./icons/plansza.png")
    image = pygame.transform.scale(
        image, (screen.get_rect().width, screen.get_rect().height))
    screen.blit(image, screen.get_rect())


def draw_pieces(screen: pygame.Surface, board: Board) -> list[PieceSprite]:
    sprites: list[PieceSprite] = []
    for pos in board.black_pieces:
        piece = board.board[pos].piece
        sprites.append(PieceSprite(pygame.image.load(f"./icons/black-{piece.name.lower()}.png"),
                       *position_to_coordinates(screen.get_rect(), pos)))

    for pos in board.white_pieces:
        piece = board.board[pos].piece
        sprites.append(PieceSprite(pygame.image.load(f"./icons/white-{piece.name.lower()}.png"),
                       *position_to_coordinates(screen.get_rect(), pos)))

    return sprites


F_1_POSITION_PERCENT: tuple[float, float] = (0.455, 1)
HEIGHT_STEP_PERCENT = -0.09
WIDTH_STEP_PERCENT = -0.087


def position_to_coordinates(screen_size: pygame.Rect, pos: position) -> tuple[int, int]:
    height_step = HEIGHT_STEP_PERCENT*screen_size.height
    width_step = WIDTH_STEP_PERCENT*screen_size.width

    return int(F_1_POSITION_PERCENT[0]*screen_size.width+(pos.col-5)*width_step), \
        int(F_1_POSITION_PERCENT[1]*screen_size.height +
            (pos.row+abs(pos.col-5)/2)*height_step)


def draw_regular_polygon(surface: pygame.Surface, color: pygame.Color, vertex_count: int, radius: int, position: tuple[int, int], width: int = 0):
    n, r = vertex_count, radius
    x, y = position
    pygame.draw.polygon(surface, color, [
        (x + r * cos(2 * pi * i / n), y + r * sin(2 * pi * i / n))
        for i in range(n)
    ], width)
