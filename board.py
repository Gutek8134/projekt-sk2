import enum
from typing import NamedTuple, Iterable
from socket import socket
import pygame


class Color(enum.Enum):
    White = 0
    Black = 1
    NoColor = 2


class Piece(enum.Enum):
    King = 0
    Queen = 1
    Rook = 2
    Bishop = 3
    Knight = 4
    Pawn = 5
    NoPiece = 6


position = NamedTuple("position", [("col", int), ("row", int)])


def pos_repr(self: position) -> str:
    return columns[self.col] + str(self.row)


position.__repr__ = pos_repr

field = NamedTuple("field", [("pos", position),
                   ("color", Color), ("piece", Piece)])


def field_repr(self: field) -> str:
    return f"{self.pos}: {self.color.name} {self.piece.name}"


field.__repr__ = field_repr

king_positions = {position(6, 1)}
queen_positions = {position(4, 1)}
rook_positions = {position(2, 1), position(8, 1)}
bishop_positions = {position(5, 1), position(5, 2), position(5, 3)}
knight_positions = {position(3, 1), position(7, 1)}
pawn_positions = {position(1, 1), position(2, 2), position(3, 3), position(4, 4), position(
    5, 5), position(6, 4), position(7, 3), position(8, 2), position(9, 1)}

col_lengths: dict[int, int] = {
    0: 6,
    1: 7,
    2: 8,
    3: 9,
    4: 10,
    5: 11,
    6: 10,
    7: 9,
    8: 8,
    9: 7,
    10: 6}

row_lengths: dict[int, int] = {1: 11,
                               2: 11,
                               3: 11,
                               4: 11,
                               5: 11,
                               6: 11,
                               7: 9,
                               8: 7,
                               9: 5,
                               10: 3,
                               11: 1}

columns = "abcdefghijk"
piece_symbols = {"K": Piece.King, "Q": Piece.Queen, "B": Piece.Bishop,
                 "R": Piece.Rook, "N": Piece.Knight, "P": Piece.Pawn}


def symmetric_position(pos: position) -> position:
    col, row = pos

    return position(col, col_lengths[col] - row + 1)


class Board:
    board: dict[position, field]
    white_king_position: position
    black_king_position: position
    white_pieces: set[position]
    black_pieces: set[position]

    all_positions: set[position]
    last_move: tuple[position, position, Piece, Color]
    last_sprite_removed: pygame.sprite.Sprite | None

    cheats = True

    def __init__(self, client_socket: socket) -> None:
        self.current_turn: Color = Color.Black
        self.game_on: bool = False
        self.awaiting_approval: bool = False
        self.client_socket = client_socket
        self.sprites: dict[position, pygame.sprite.Sprite] = {}
        self.last_sprite_removed = None
        self.player_color: Color = Color.NoColor
        self.reset_board()

    def reset_board(self) -> None:
        self.board = {}
        self.white_pieces = set()
        self.black_pieces = set()
        self.all_positions = set()

        for row in range(1, 12):
            row_length = row_lengths[row]
            distance_from_f = (row_length - 1) // 2

            for column_index in range(5 - distance_from_f, 5 + distance_from_f+1):
                pos = position(column_index, row)
                self.all_positions.add(pos)
                self.board[pos] = field(pos, Color.NoColor, Piece.NoPiece)

        # region fill board
        for pos in king_positions:
            self.board[pos] = field(pos, Color.White, Piece.King)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.King)

            self.white_king_position = pos
            self.black_king_position = sym_pos
            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        for pos in queen_positions:
            self.board[pos] = field(pos, Color.White, Piece.Queen)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.Queen)

            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        for pos in rook_positions:
            self.board[pos] = field(pos, Color.White, Piece.Rook)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.Rook)

            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        for pos in bishop_positions:
            self.board[pos] = field(pos, Color.White, Piece.Bishop)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.Bishop)

            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        for pos in knight_positions:
            self.board[pos] = field(pos, Color.White, Piece.Knight)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.Knight)

            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        for pos in pawn_positions:
            self.board[pos] = field(pos, Color.White, Piece.Pawn)

            sym_pos = symmetric_position(pos)
            self.board[sym_pos] = field(sym_pos, Color.Black, Piece.Pawn)

            self.white_pieces.add(pos)
            self.black_pieces.add(sym_pos)

        # endregion fill board

    def apply_move(self, from_pos: position, to_pos: position) -> None:
        if self.board[from_pos].color == Color.White:
            self.white_pieces.remove(from_pos)
            self.white_pieces.add(to_pos)
            if self.board[from_pos].piece == Piece.King:
                self.white_king_position = to_pos

        if self.board[from_pos].color == Color.Black:
            self.black_pieces.remove(from_pos)
            self.black_pieces.add(to_pos)
            if self.board[from_pos].piece == Piece.King:
                self.black_king_position = to_pos

        if self.board[to_pos].piece == Piece.King:
            self.game_on = False

        if self.board[to_pos].color == Color.White:
            self.white_pieces.remove(to_pos)

        if self.board[to_pos].color == Color.Black:
            self.black_pieces.remove(to_pos)

        self.board[to_pos] = self.board[from_pos]
        self.board[from_pos] = field(from_pos, Color.NoColor, Piece.NoPiece)
        self.current_turn = Color.White if self.current_turn == Color.Black else Color.Black

        if to_pos in self.sprites:
            self.sprites[to_pos].visible = False  # type: ignore

        self.last_sprite_removed = None
        self.sprites[from_pos].pos = to_pos  # type: ignore
        self.sprites[to_pos] = self.sprites[from_pos]
        self.sprites.pop(from_pos)

    def move(self, from_pos: position, to_pos: position) -> None:
        if self.awaiting_approval:
            return

        if self.board[from_pos].color == Color.White:
            self.white_pieces.remove(from_pos)
            self.white_pieces.add(to_pos)
            if self.board[from_pos].piece == Piece.King:
                self.white_king_position = to_pos

        if self.board[from_pos].color == Color.Black:
            self.black_pieces.remove(from_pos)
            self.black_pieces.add(to_pos)
            if self.board[from_pos].piece == Piece.King:
                self.black_king_position = to_pos

        if self.board[to_pos].piece == Piece.King:
            self.game_on = False

        if self.board[to_pos].color == Color.White:
            self.white_pieces.remove(to_pos)

        if self.board[to_pos].color == Color.Black:
            self.black_pieces.remove(to_pos)

        self.last_move = (from_pos, to_pos,
                          self.board[to_pos].piece, self.board[to_pos].color)
        self.board[to_pos] = self.board[from_pos]
        self.board[from_pos] = field(from_pos, Color.NoColor, Piece.NoPiece)
        self.current_turn = Color.White if self.current_turn == Color.Black else Color.Black

        if to_pos in self.sprites:
            self.sprites[to_pos].visible = False  # type: ignore

        self.last_sprite_removed = self.sprites.get(to_pos, None)
        self.sprites[from_pos].pos = to_pos  # type: ignore
        self.sprites[to_pos] = self.sprites[from_pos]
        self.sprites.pop(from_pos)

        print("SEND:", f"move {from_pos} {to_pos}")
        self.client_socket.send(f"move {from_pos} {to_pos}".encode())

        self.awaiting_approval = True

    def retract_last_move(self) -> None:
        from_pos, to_pos, piece, color = self.last_move

        if self.board[to_pos].color == Color.White:
            self.white_pieces.remove(to_pos)
            self.white_pieces.add(from_pos)
            if self.board[to_pos].piece == Piece.King:
                self.white_king_position = from_pos

        if self.board[to_pos].color == Color.Black:
            self.black_pieces.remove(to_pos)
            self.black_pieces.add(from_pos)
            if self.board[to_pos].piece == Piece.King:
                self.black_king_position = to_pos

        if color == Color.White:
            self.white_pieces.add(to_pos)
        if color == Color.Black:
            self.black_pieces.add(to_pos)

        self.board[from_pos] = self.board[to_pos]
        self.board[to_pos] = field(to_pos, color, piece)

        self.sprites[from_pos] = self.sprites[to_pos]
        self.sprites[from_pos].pos = from_pos  # type: ignore

        if self.last_sprite_removed is not None:
            self.last_sprite_removed.visible = True  # type: ignore
            self.sprites[to_pos] = self.last_sprite_removed

        self.current_turn = self.player_color
        if piece == Piece.King:
            self.game_on = True

    def get_possible_moves(self, pos: position) -> Iterable[position]:
        if Board.cheats:
            return self.all_positions.difference({pos})
        player_color = self.board[pos].color
        return {
            Piece.King: self.get_possible_king_moves,
            Piece.Queen: self.get_possible_queen_moves,
            Piece.Bishop: self.get_possible_bishop_moves,
            Piece.Rook: self.get_possible_rook_moves,
            Piece.Knight: self.get_possible_knight_moves,
            Piece.Pawn: self.get_possible_pawn_moves,
        }[self.board[pos].piece](pos, player_color)

    def get_possible_king_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = [
            position(pos.col-1, pos.row+1),
            position(pos.col, pos.row+1),
            position(pos.col+1, pos.row+1),
            position(pos.col-1, pos.row),
            position(pos.col+1, pos.row),
        ]

        moves = [self.abs_move(pos, move) for move in moves]
        moves = [
            move for move in moves if move in self.all_positions and not self.position_under_attack(move, player_color) and self.board[move].color != player_color]
        return moves

    def get_possible_queen_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = []

        moves.extend(self.generate_orthogonal_moves(pos, player_color))
        moves.extend(self.generate_diagonal_moves(pos, player_color))

        return moves

    def get_possible_bishop_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = []

        moves.extend(self.generate_diagonal_moves(pos, player_color))

        return moves

    def get_possible_rook_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = []

        moves.extend(self.generate_orthogonal_moves(pos, player_color))

        return moves

    def get_possible_knight_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = [
            position(pos.col-3, pos.row-1),
            position(pos.col-3, pos.row-2),
            position(pos.col-2, pos.row+1),
            position(pos.col-2, pos.row-3),
            position(pos.col-1, pos.row+2),
            position(pos.col-1, pos.row-3),
            position(pos.col+1, pos.row+2),
            position(pos.col+1, pos.row-3),
            position(pos.col+2, pos.row+1),
            position(pos.col+2, pos.row-3),
            position(pos.col+3, pos.row-1),
            position(pos.col+3, pos.row-2),
        ]

        moves = [self.abs_move(pos, move) for move in moves]
        moves = [
            move for move in moves if move in self.all_positions and self.board[move].color != player_color]

        return moves

    def get_possible_pawn_moves(self, pos: position, player_color: Color) -> list[position]:
        moves_forward: list[position] = [position(
            pos.col, pos.row + (1 if player_color == Color.White else -1))]
        if player_color == Color.Black and pos.row == 7:
            moves_forward.append(position(pos.col, pos.row-2))
        elif player_color == Color.White and pos.row == 5-abs(pos.col-5):
            moves_forward.append(position(pos.col, pos.row+2))
        moves_forward = [self.abs_move(pos, move) for move in moves_forward]

        moves_sideways: list[position] = [position(pos.col-1, pos.row), position(pos.col+1, pos.row)] if player_color == Color.White else [
            position(pos.col-1, pos.row-1), position(pos.col+1, pos.row-1)]

        moves_sideways = [self.abs_move(pos, move) for move in moves_sideways]

        moves = [move for move in moves_sideways if self.board[move].color == (
            Color.White if player_color == Color.Black else Color.Black)]

        for move in moves_forward:
            if move in self.all_positions and self.board[move].color == Color.NoColor:
                moves.append(move)

        return moves

    def position_under_attack(self, pos: position, player_color: Color) -> bool:
        for piece in (self.black_pieces if player_color == Color.Black else self.white_pieces):
            if pos in self.get_possible_moves(piece):
                return True

        return False

    def abs_move(self, from_pos: position, to_pos: position) -> position:
        """
        Convert position row from f-column relative to absolute positioning
        """
        # 5==f since python doesn't treat characters as numbers
        if from_pos.col == 5:
            return to_pos

        if from_pos.col > 5 and to_pos.col < from_pos.col:
            return position(to_pos.col, to_pos.row +
                            abs(max(to_pos.col, 5)-from_pos.col))

        if from_pos.col < 5 and to_pos.col > from_pos.col:
            return position(to_pos.col, to_pos.row +
                            abs(min(to_pos.col, 5)-from_pos.col))

        return to_pos

    def generate_orthogonal_moves(self, pos: position, player_color: Color) -> list[position]:
        """
        Absolute positioning
        """
        moves: list[position] = []

        # Up-down
        for row in range(pos.row+1, col_lengths[pos.col]+1):
            orthogonal_pos = position(pos.col, row)
            if self.board[orthogonal_pos].piece != Piece.NoPiece:
                if self.board[orthogonal_pos].color != player_color:
                    moves.append(orthogonal_pos)
                break

            moves.append(orthogonal_pos)

        for row in range(pos.row-1, 0, -1):
            orthogonal_pos = position(pos.col, row)
            if self.board[orthogonal_pos].piece != Piece.NoPiece:
                if self.board[orthogonal_pos].color != player_color:
                    moves.append(orthogonal_pos)
                break

            moves.append(orthogonal_pos)

        # With the flow
        if pos.col <= 5:
            # Left-up
            for col in range(pos.col-1, (11-row_lengths[pos.row])//2-1, -1):
                orthogonal_pos = position(col, pos.row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    break

                moves.append(orthogonal_pos)
            # Left-down
            for col, row in zip(range(pos.col-1, (11-row_lengths[pos.row])//2-1, -1),
                                range(pos.row-1, 0, -1)):
                orthogonal_pos = position(col, row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    break

                moves.append(orthogonal_pos)

        if pos.col >= 5:
            # Right-up
            for col in range(pos.col+1, (11+row_lengths[pos.row])//2+1, 1):
                orthogonal_pos = position(col, pos.row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    break

                moves.append(orthogonal_pos)

            # Right-down
            for col, row in zip(range(pos.col+1, (11+row_lengths[pos.row])//2+1, 1),
                                range(pos.row-1, 0, -1)):
                orthogonal_pos = position(col, row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    break

                moves.append(orthogonal_pos)

        # Against the flow
        skip: bool = False
        if pos.col > 5:
            # Left-up
            # Until f, inclusive
            for col, row in zip(range(pos.col-1, 4, -1),
                                range(pos.row+1, pos.row+pos.col-5+1, 1)):
                orthogonal_pos = position(col, row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    skip = True
                    break

                moves.append(orthogonal_pos)
            if not skip:
                for col in range(4, (11-row_lengths[pos.row])//2-1, -1):
                    orthogonal_pos = position(col, pos.row+pos.col-5)
                    if self.board[orthogonal_pos].piece != Piece.NoPiece:
                        if self.board[orthogonal_pos].color != player_color:
                            moves.append(orthogonal_pos)
                        break

                    moves.append(orthogonal_pos)

            skip = False

            # Left-down
            # Until f, inclusive
            for col in range(pos.col-1, 4, -1):
                orthogonal_pos = position(col, pos.row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    skip = True
                    break

                moves.append(orthogonal_pos)

            if not skip:
                for col, row in zip(range(4, (11-row_lengths[pos.row])//2-1, -1),
                                    range(pos.row-1, 0, -1)):

                    orthogonal_pos = position(col, row)
                    if self.board[orthogonal_pos].piece != Piece.NoPiece:
                        if self.board[orthogonal_pos].color != player_color:
                            moves.append(orthogonal_pos)
                        break

                    moves.append(orthogonal_pos)

        if pos.col < 5:
            # Right-up
            skip = False
            # Until f, inclusive
            for col, row in zip(range(pos.col+1, 6, 1),
                                range(pos.row+1, pos.row-pos.col+5+1, 1)):
                orthogonal_pos = position(col, row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    skip = True
                    break

                moves.append(orthogonal_pos)
            if not skip:
                for col in range(6, (11+row_lengths[pos.row])//2+1, 1):
                    orthogonal_pos = position(col, pos.row-pos.col+5)
                    if self.board[orthogonal_pos].piece != Piece.NoPiece:
                        if self.board[orthogonal_pos].color != player_color:
                            moves.append(orthogonal_pos)
                        break

                    moves.append(orthogonal_pos)

            # Right-down
            skip = False
            # Until f, inclusive
            for col in range(pos.col+1, 6, 1):
                orthogonal_pos = position(col, pos.row)
                if self.board[orthogonal_pos].piece != Piece.NoPiece:
                    if self.board[orthogonal_pos].color != player_color:
                        moves.append(orthogonal_pos)
                    skip = True
                    break

                moves.append(orthogonal_pos)

            if not skip:
                for col, row in zip(range(6, (11+row_lengths[pos.row])//2+1, 1),
                                    range(pos.row-1, 0, -1)):
                    orthogonal_pos = position(col, row)
                    if self.board[orthogonal_pos].piece != Piece.NoPiece:
                        if self.board[orthogonal_pos].color != player_color:
                            moves.append(orthogonal_pos)
                        break

                    moves.append(orthogonal_pos)

        return moves

    def generate_diagonal_moves(self, pos: position, player_color: Color) -> list[position]:
        moves: list[position] = []

        # With the flow
        if pos.col <= 5:
            # Left-up
            for col, row in zip(range(pos.col - 1, -1, -1),
                                range(pos.row + 1, 12, 1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
            # Left
            for col, row in zip(range(pos.col - 2, -1, -2),
                                range(pos.row - 1, 0, -1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
            # Left-down
            for col, row in zip(range(pos.col - 1, -1, -1),
                                range(pos.row - 2, 0, -2)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
        if pos.col >= 5:
            # Right-up
            for col, row in zip(range(pos.col + 1, 11, 1),
                                range(pos.row + 1, 12, 1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
            # Right
            for col, row in zip(range(pos.col + 2, 1, 2),
                                range(pos.row - 1, 0, -1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
            # Right-down
            for col, row in zip(range(pos.col + 1, 11, 1),
                                range(pos.row - 2, 0, -2)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    break

                moves.append(diagonal_pos)
            pass

        # Against the flow:
        if pos.col > 5:
            # Left-up
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col - 1, 5, -1),
                                range(pos.row + 2, 12, 2)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(5, -1, -1),
                                    range(pos.row + (pos.col-5)*2, 12, 1)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)

            # Left
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col - 2, 5, -2),
                                range(pos.row + 1, 12, 1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(4 if pos.col % 2 == 0 else 5, -1, -2),
                                    range(pos.row + (pos.col - 5)//2, 0, -1)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)
            # Left-down
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col - 1, 5, -1),
                                range(pos.row - 1, 0, -1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(5, -1, -1),
                                    range(pos.row - (pos.col - 5), 0, -2)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)

        if pos.col < 5:
            # Right-up
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col + 1, 5, 1),
                                range(pos.row + 2, 12, 2)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(5, 11, 1),
                                    range(pos.row + (5-pos.col)*2, 12, 1)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)

            # Right
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col + 2, 5, 2),
                                range(pos.row + 1, 12, 1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(6 if pos.col % 2 == 0 else 5, 11, 2),
                                    range(pos.row + (5-pos.col)//2, 0, -1)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)
            # Right-down
            skip = False
            # Until f, NOT inclusive
            for col, row in zip(range(pos.col + 1, 5, 1),
                                range(pos.row - 1, 0, -1)):
                diagonal_pos = position(col, row)
                if diagonal_pos not in self.all_positions:
                    break
                if self.board[diagonal_pos].piece != Piece.NoPiece:
                    if self.board[diagonal_pos].color != player_color:
                        moves.append(diagonal_pos)
                    skip = True
                    break

                moves.append(diagonal_pos)

            if not skip:
                for col, row in zip(range(5, 11, 1),
                                    range(pos.row - (5-pos.col), 0, -2)):
                    diagonal_pos = position(col, row)
                    if diagonal_pos not in self.all_positions:
                        break
                    if self.board[diagonal_pos].piece != Piece.NoPiece:
                        if self.board[diagonal_pos].color != player_color:
                            moves.append(diagonal_pos)
                        break

                    moves.append(diagonal_pos)

        return moves

    def load(self, state: str):
        for pos in self.all_positions:
            self.board[pos] = field(pos, Color.NoColor, Piece.NoPiece)

        self.white_pieces.clear()
        self.black_pieces.clear()

        print(state)
        for line in state.splitlines():
            line = line.split()
            print(line)
            if line[0][0] == "W":
                pos = position(columns.index(line[1][0]), int(line[1][1:]))
                self.white_pieces.add(pos)
                self.board[pos] = field(
                    pos, Color.White, piece_symbols[line[0][1]])
                if piece_symbols[line[0][1]] == Piece.King:
                    self.white_king_position = pos
            elif line[0][0] == "B":
                pos = position(columns.index(line[1][0]), int(line[1][1:]))
                self.black_pieces.add(pos)
                self.board[pos] = field(
                    pos, Color.Black, piece_symbols[line[0][1]])
                if piece_symbols[line[0][1]] == Piece.King:
                    self.black_king_position = pos
