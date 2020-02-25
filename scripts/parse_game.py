#!/usr/bin/env python3

import sys
import struct
import numpy as np
from enum import IntEnum
from termcolor import colored

SIZE = 5
STATE = "30s5sB10s10sIIBBBBb"

T = ["  ", "88", "  ", "XX", "<>", "1 ", ".."]
C = ["blue", "red", "red", "white", "blue", "grey", None]
B = ["on_blue", "on_yellow", "on_red", "on_grey", "on_white", "on_white", None]
A = [["bold"], ["bold"], ["bold"], ["bold"], ["bold", "dark"], ["bold", "dark"], None]

class Tile(IntEnum):
    BLUE = 0
    YELLOW = 1
    RED = 2
    BLACK = 3
    WHITE = 4
    FIRST = 5
    NONE = 6

    def __str__(self):
        i = self.value
        if i == Tile.NONE:
            return T[i]
        return colored(T[i], C[i], B[i], A[i])


class Board:
    def __init__(self):
        self.wall = np.zeros((SIZE, SIZE), dtype=np.uint8) + Tile.NONE
        self.lines = []
        for i in range(SIZE):
            self.lines.append(np.repeat(Tile.NONE, i + 1))
        self.floorline = 0


    def parse(self, left, wall, floor):
        self.wall[:] = Tile.NONE
        self.floorline = floor
        for i in range(SIZE):
            self.lines[i][:] = Tile.NONE
            t, n = struct.unpack("BB", left[i*2:i*2+2])
            j = 0
            while n > 0:
                self.lines[i][j] = t 
                n -= 1
                j += 1

        def tile(row, col): 
            return (SIZE + col - row) % SIZE

        for i in range(SIZE):
            for j in range(SIZE):
                n = i * SIZE + j
                if (wall >> n) & 1:
                    self.wall[i, j] = tile(i, j)


    def __str__(self):
        s = ""
        for i in range(SIZE):
            s += f" {i+1} "
            s += "   " * (SIZE - i - 1)
            s += " ".join(str(Tile(t)) for t in self.lines[i])
            s += " > "
            s += " ".join(str(Tile(t)) for t in self.wall[i])
            s += "\n"
        s += f" F {self.floorline}\n" 
        return s


class State:
    def __init__(self):
        self.factories = np.zeros((5, 4), dtype=np.uint8) + Tile.NONE
        self.center = np.zeros(16, dtype=np.uint8) + Tile.NONE
        self.bag = np.zeros(5, dtype=np.uint8)
        self.turn = 0
        self.first = -1
        self.boards = [Board(), Board()]
        self.scores = [0, 0]


    def parse(self, buf):
        self.__init__()
        c, b, t, l1, l2, w1, w2, f1, f2, s1, s2, f = struct.unpack(STATE, buf)
        for i in range(5):
            self.bag[i] = b[i]
        self.turn = t
        self.first = f
        for i in range(5):
            tiles = struct.unpack("BBBBB", c[i*5:i*5+5])
            j = 0
            for t,n in enumerate(tiles):
                while n > 0:
                    self.factories[i, j] = t 
                    n -= 1
                    j += 1
        tiles = struct.unpack("BBBBB", c[25:])
        j = 0
        if self.first == -1:
            self.center[0] = Tile.FIRST
            j = 1
        for t, n in enumerate(tiles):
            while n > 0:
                self.center[j] = t 
                n -= 1
                j += 1
        p = t if t == 0 else 1
        self.scores[p] = s1
        self.scores[1^p] = s2
        self.boards[p].parse(l1, w1, f1)
        self.boards[1^p].parse(l2, w2, f2)


    def __str__(self):
        s = ""
        for i in range(5):
            s += f"   {i+1}      "
        s += "\n"
        for f in self.factories:
            s += f" {str(Tile(f[0]))} {str(Tile(f[1]))}    "
        s += "\n"
        for f in self.factories:
            s += f" {str(Tile(f[2]))} {str(Tile(f[3]))}    "
        s += "\n\n C ["
        s += " ".join([str(Tile(i)) for i in self.center if i != Tile.NONE])
        s += "]\n\n"
        s += " B "
        for i in range(5):
            s += f"{self.bag[i]}:{str(Tile(i))} "
        s += "\n\n"
        s += f" Player 1: {self.scores[0]:3d}"
        s += " " * 25
        s += f"Player 2: {self.scores[1]:3d}\n" 
        b1 = str(self.boards[0]).split('\n')
        b2 = str(self.boards[1]).split('\n')
        for i in range(SIZE):
            s += b1[i] + "    " + b2[i] + "\n"
        s += b1[5] + (" "*34) + b2[5] + "\n"
        return s
                


if __name__ == "__main__":
    with open(sys.argv[1], "rb") as f:
        buf = f.read()

    size = 69 + 180*4 + 1
    n = len(buf) // size
    assert(len(buf) % size == 0)
    assert(struct.calcsize(STATE) == 69)

    state = State()
    print("-" * 69, end='')
    print(f"001")
    for i in range(n):
        idx = i * size
        state.parse(buf[idx:idx+69])
        print(state)
        if i < n - 1:
            print("-" * 69, end='')
            print(f"{i+2:03}")
