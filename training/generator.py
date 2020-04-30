import glob
import random
import tensorflow as tf
import numpy as np
import struct

NUM_PLANES = 49
IMG_SIZE = 5
NUM_MOVES = 180
STATE = "30s5sB10s10s4s4sBBBBb720sb"

class Generator(tf.keras.utils.Sequence):
    def __init__(self, inputdir, shuffle, batchsize):
        self.record_struct = struct.Struct(STATE)
        self.shuffle = shuffle
        self.batch_size = batchsize
        self.records = []

        # load all games into memory (~56K per game) and store them per record
        # for optimal shuffling.
        filenames = glob.glob(str(inputdir) + "/*.bin")
        recordsize = self.record_struct.size
        for name in filenames:
            with open(name, "rb") as f:
                data = f.read()
            n = len(data)
            assert n % recordsize == 0
            for i in range(0, n, recordsize):
                self.records.append(data[i:i+recordsize])
        print(f"Loaded {len(self.records)} records shuffle:{shuffle}")


    def __iter__(self):
        if self.shuffle:
            random.shuffle(self.records)
        return self


    def __len__(self):
        return len(self.records) // self.batch_size


    def __getitem__(self, index):
        x, y = [], ([], [])
        for i in range(self.batch_size):
            s, p, z = self._create_planes(self.records[index * self.batch_size + i])
            x.append(s)
            y[0].append(p)
            y[1].append(z)
        return np.transpose(np.array(x), [0, 2, 3, 1]), (np.array(y[0]), np.array(y[1]))


    def on_epoch_end(self):
        if self.shuffle:
            random.shuffle(self.records)


    def _create_planes(self, data):
        """
        1 + 1 + 5 + 5*4 + 15 + 1 + 1 + 1 + 1 + 1 + 1 + 1 = 49
        |   |   |   |     |    |   |   |   |   |   |   |
        |   |   |   |     |    |   |   |   |   |   |   them floor: v in {0,...,7}
        |   |   |   |     |    |   |   |   |   |   them wall: v in {0, 1}
        |   |   |   |     |    |   |   |   |   them left: v in {0,...,5}
        |   |   |   |     |    |   |   |   us floor: v in {0,...,7}
        |   |   |   |     |    |   |   us wall: v in {0, 1}
        |   |   |   |     |    |   us left: v in {0,...,5}
        |   |   |   |     |    1st tile: v in {-1, 0, 1}
        |   |   |   |     center: v in {0,...,5}
        |   |   |   factories: v in {0,...,5}
        |   |   bag: v in {0,...,20}
        |   them score: v in {0,...,255}
        us score: v in {0,...,255}

        All planes are normalized between [0, 1]
        """
        c, b, t, l1, l2, w1, w2, f1, f2, s1, s2, f, probs, winner = self.record_struct.unpack(data)

        # make sure that the player who's turn it is, is always player 1. This
        # ensures we create the planes from the current player's perspective.
        if t == 1:
            l1, l2 = l2, l1
            w1, w2 = w2, w1
            f1, f2 = f2, f1
            s1, s2 = s2, s1

        # create buffer
        planes = np.zeros((NUM_PLANES, IMG_SIZE, IMG_SIZE), dtype=np.float32)

        # scores
        index = 0
        planes[index, :, :] = s1 / 255
        index += 1
        planes[index, :, :] = s2 / 255
        index += 1

        # bag
        for i in range(5):
            planes[index, :, :] = b[i] / 20
            index += 1

        # factories
        for i in range(5):
            tiles = struct.unpack("BBBBB", c[i*5:i*5+5])
            empty = 4
            for tile, n in enumerate(tiles):
                for _ in range(n):
                    planes[index, :, :] = (tile + 1) / 5
                    empty -= 1
                    index += 1
            for _ in range(empty):
                index += 1

        # center
        tiles = struct.unpack("BBBBB", c[25:])
        empty = 15
        for tile, n in enumerate(tiles):
            for _ in range(n):
                planes[index, :, :] = (tile + 1) / 5
                empty -= 1
                index += 1
        for _ in range(empty):
            index += 1

        # first tile
        planes[index, :, :] = (f + 1) / 2
        index += 1

        # p1 left
        for i in range(5):
            tile, n = struct.unpack("BB", l1[i*2:i*2+2])
            for j in range(n):
                planes[index, i, j] = (tile + 1) / 5
        index += 1

        # p1 wall
        planes[index, :, :] = np.unpackbits(np.frombuffer(w1, dtype=np.uint8))[7:].reshape(IMG_SIZE, IMG_SIZE).astype(np.float32)
        index += 1

        # p1 floor
        planes[index, :, :] = f1 / 7
        index += 1

        # p2 left
        for i in range(5):
            tile, n = struct.unpack("BB", l2[i*2:i*2+2])
            for j in range(n):
                planes[index, i, j] = (tile + 1) / 5
        index += 1

        # p2 wall
        planes[index, :, :] = np.unpackbits(np.frombuffer(w2, dtype=np.uint8))[7:].reshape(IMG_SIZE, IMG_SIZE).astype(np.float32)
        index += 1

        # p2 floor
        planes[index, :, :] = f2 / 7
        index += 1

        assert index == NUM_PLANES

        return planes, np.frombuffer(probs, dtype=np.float32), np.float32(winner)
