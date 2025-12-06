import pygame
from math import sin, cos, pi
from board import Board, position

WHITE = 255, 255, 255
CLICKABLE_ICON = pygame.image.load("./icons/clickable.png")


class MoveSprite(pygame.sprite.Sprite):
    def __init__(self, pos: position, controlled_sprite: "PieceSprite", screen_size: pygame.Rect):
        super().__init__()
        self.image = CLICKABLE_ICON
        self.rect = self.image.get_rect()
        self.rect.x, self.rect.y = position_to_coordinates(screen_size, pos)
        self.visible = True
        self.pos = pos
        self.controlled_sprite = controlled_sprite

    def update(self, events: list[pygame.event.Event]):
        for event in events:
            if event.type == pygame.MOUSEBUTTONUP:
                if self.rect.collidepoint(event.pos):
                    self.controlled_sprite.board.move(
                        self.controlled_sprite.pos, self.pos)
                    self.controlled_sprite.rect.x, self.controlled_sprite.rect.y = self.rect.x, self.rect.y
                    self.controlled_sprite.pos = self.pos
                    self.controlled_sprite.clickable_moves_group.empty()


class PieceSprite(pygame.sprite.Sprite):
    clickable_moves_group: pygame.sprite.Group

    def __init__(self, image: pygame.Surface, x: int, y: int, board: Board, pos: position, screen_size: pygame.Rect):
        super().__init__()
        self.image = image
        self.rect = self.image.get_rect()
        self.rect.x = x
        self.rect.y = y
        self.visible = True
        self.board = board
        self.pos = pos
        self.screen_size = screen_size

    def update(self, events: list[pygame.event.Event]):
        for event in events:
            if event.type == pygame.MOUSEBUTTONUP:
                if self.rect.collidepoint(event.pos):
                    for possible_move in self.board.get_possible_moves(self.pos):
                        PieceSprite.clickable_moves_group.add(
                            MoveSprite(possible_move, self, self.screen_size))


def draw_blank_board(screen: pygame.Surface) -> None:
    image = pygame.image.load("./icons/plansza.png")
    image = pygame.transform.scale(
        image, (screen.get_rect().width, screen.get_rect().height))
    screen.blit(image, screen.get_rect())


def draw_pieces(screen: pygame.Surface, board: Board, moves_group: pygame.sprite.Group) -> list[PieceSprite]:
    PieceSprite.clickable_moves_group = moves_group
    sprites: list[PieceSprite] = []
    for pos in board.black_pieces:
        piece = board.board[pos].piece
        sprite = PieceSprite(pygame.image.load(f"./icons/black-{piece.name.lower()}.png"),
                             *position_to_coordinates(screen.get_rect(), pos), board, pos, screen.get_rect())
        sprites.append(sprite)

    for pos in board.white_pieces:
        piece = board.board[pos].piece
        sprite = PieceSprite(pygame.image.load(f"./icons/white-{piece.name.lower()}.png"),
                             *position_to_coordinates(screen.get_rect(), pos), board, pos, screen.get_rect())
        sprites.append(sprite)

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
