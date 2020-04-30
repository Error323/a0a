#!/usr/bin/env python3

import sys
import glob
import struct
import argparse
import random
import multiprocessing
import tensorflow as tf
import numpy as np

from pathlib import Path
from tensorflow.keras.layers import Conv2D, Dense, BatchNormalization, AvgPool2D, Add, Activation, Flatten
from tensorflow.keras import Model, Input
from generator import Generator

BATCH_SIZE = 512
NUM_PLANES = 49
IMG_SIZE = 5
NUM_MOVES = 180
STATE = "30s5sB10s10s4s4sBBBBb720sb"
STATESIZE = struct.calcsize(STATE)
SHUFFLE_BUFFER_SIZE = 100000

reg = tf.keras.regularizers.l2(l=1e-4)

# 1d planes for easy indexing
flat_planes = []
for i in range(256):
    flat_planes.append(np.zeros(25, dtype=np.float32) + i)
    

def create_planes(data):
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
    """
    buf = struct.unpack(STATE, data.numpy())
    c, b, t, l1, l2, w1, w2, f1, f2, s1, s2, f, probs, winner = buf

    # scores
    planes = flat_planes[s1].tobytes() + flat_planes[s2].tobytes()

    # bag
    for i in range(5):
        planes += flat_planes[b[i]].tobytes()

    # factories
    for i in range(5):
        tiles = struct.unpack("BBBBB", c[i*5:i*5+5])
        empty = 4
        for t, n in enumerate(tiles):
            for _ in range(n):
                planes += flat_planes[t + 1].tobytes()
                empty -= 1
        for _ in range(empty):
            planes += flat_planes[0].tobytes()

    # center
    tiles = struct.unpack("BBBBB", c[25:])
    empty = 15
    for t, n in enumerate(tiles):
        for _ in range(n):
            planes += flat_planes[t + 1].tobytes()
            empty -= 1
    for _ in range(empty):
        planes += flat_planes[0].tobytes()

    # first tile
    planes += (np.zeros(25, dtype=np.float32) + f).tobytes()

    # p1 left
    plane = np.zeros((5, 5), dtype=np.float32)
    for i in range(5):
        t, n = struct.unpack("BB", l1[i*2:i*2+2])
        for j in range(n):
            plane[i, j] = t + 1
    planes += plane.tobytes()

    # p1 wall
    planes += np.unpackbits(np.frombuffer(w1, dtype=np.uint8))[7:].astype(np.float32).tobytes()

    # p1 floor
    planes += flat_planes[f1].tobytes()

    # p2 left
    plane = np.zeros((5, 5), dtype=np.float32)
    for i in range(5):
        t, n = struct.unpack("BB", l2[i*2:i*2+2])
        for j in range(n):
            plane[i, j] = t + 1
    planes += plane.tobytes()

    # p2 wall
    planes += np.unpackbits(np.frombuffer(w2, dtype=np.uint8))[7:].astype(np.float32).tobytes()

    # p2 floor
    planes += flat_planes[f2].tobytes()

    assert len(planes) == IMG_SIZE * IMG_SIZE * NUM_PLANES * 4

    winner = float(winner)
    winner = struct.pack('f', winner)

    return planes, probs, winner
    

def parse_function(data):
    """converts raw bytes containing (s, pi, z) to tensors"""
    [planes, probs, winner] = tf.py_function(func=create_planes, inp=[data], Tout=(tf.string, tf.string, tf.string) )
    s = tf.io.decode_raw(planes, tf.float32)
    pi = tf.io.decode_raw(probs, tf.float32)
    z = tf.io.decode_raw(winner, tf.float32)
    s = tf.reshape(s, (IMG_SIZE, IMG_SIZE, NUM_PLANES))
    pi = tf.reshape(pi, (NUM_MOVES,))
    z = tf.reshape(z, (1,))
    return s, (pi, z)


def create_dataset(inputdir, shuffle):
    gen = Generator(str(inputdir), shuffle, STATESIZE)
    ds = tf.data.Dataset.from_generator(lambda: (x for x in gen), tf.string)
    ds = ds.map(parse_function, num_parallel_calls=8)
    ds = ds.batch(BATCH_SIZE)
    return ds


def conv(x, filters, kernel_size, strides=(1, 1)):
    x = Conv2D(filters, kernel_size, strides, padding="same", kernel_regularizer=reg, use_bias=False)(x)
    x = BatchNormalization(scale=False, beta_regularizer=reg)(x)
    x = Activation("relu")(x)
    return x


def res_block(inputs, filters, kernel_size):
    x = conv(inputs, filters, kernel_size)
    x = Conv2D(filters, kernel_size, (1, 1), padding="same", kernel_regularizer=reg, use_bias=False)(x)
    x = BatchNormalization(scale=False, beta_regularizer=reg)(x)
    x = Add()([x, inputs])
    x = Activation("relu")(x)
    return x


def resnet(x, blocks, filters):
    # initial convolution
    x = conv(x, filters, (3, 3))

    # residual blocks
    for i in range(blocks):
        x = res_block(x, filters, (3, 3))

    # policy head
    pi = conv(x, NUM_MOVES, (1, 1))
    pi = AvgPool2D(pool_size=(IMG_SIZE, IMG_SIZE))(pi)
    pi = Flatten(name="policy")(pi)

    # value head
    v = conv(x, 32, (1, 1))
    v = Flatten()(v)
    v = Dense(64, kernel_regularizer=reg, bias_regularizer=reg, activation="relu")(v)
    v = Dense(1, kernel_regularizer=reg, bias_regularizer=reg, activation="tanh", name="value")(v)

    return pi, v


class LogLr(tf.keras.callbacks.Callback):
    def __init__(self, steps=500):
        self.steps = steps
        self.counter = 0

    def on_train_begin(self, logs=None):
        self.counter = 0

    def on_batch_end(self, batch, logs=None):
        lr = self.model.optimizer.lr.numpy()
        if self.counter % self.steps == 0:
            tf.summary.scalar("learning rate", data=lr, step=self.counter)
        self.counter += 1


def main(args):
    dst = args.output
    wgt = dst / "weights"
    logdir = str(dst / "logs/scalars")
    checkpoint_path = str(wgt)
    dst.mkdir(parents=True, exist_ok=True)


    train_ds = create_dataset(args.input / "train", True)
    test_ds = create_dataset(args.input / "test", False)

    """
    for x, y in train_ds.take(1):
        print(x)
        print(y[0])
        print(y[1])
    """

    file_writer = tf.summary.create_file_writer(logdir + "/metrics")
    file_writer.set_as_default()
    # Create a callback that saves the model's weights
    checkpoint_cb = tf.keras.callbacks.ModelCheckpoint(filepath=checkpoint_path, verbose=1)
    # Log various metrics on tensorboard with this callback
    tensorboard_cb = tf.keras.callbacks.TensorBoard(log_dir=logdir, update_freq=1000, profile_batch=0)

    inputs = Input(shape=(IMG_SIZE, IMG_SIZE, NUM_PLANES), name="planes")
    policy, value = resnet(inputs, args.blocks, args.filters)
    model = Model(inputs, outputs=[policy, value], name=f"ResNet-{args.blocks}x{args.filters}")
    losses = {"policy": "categorical_crossentropy", "value": "mean_squared_error"}

    print(model.summary())

    # Compile and train
    model.compile(optimizer=tf.keras.optimizers.SGD(learning_rate=args.lr, momentum=0.9, nesterov=True),
            loss=losses,
            metrics=["accuracy"])

    model.fit(train_ds, 
            steps_per_epoch=args.steps,
            validation_data=test_ds, 
            validation_steps=60000//BATCH_SIZE, 
            callbacks=[tensorboard_cb, checkpoint_cb, LogLr()])

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AlphaZero Azul Trainer")
    parser.add_argument("--input", type=Path, \
            help="input directory containing test/ and train/ subdirs")
    parser.add_argument("--resume", type=Path, default=None, \
            help="resume from this model")
    parser.add_argument("--lr", type=float, \
            help="learningrate to use this session")
    parser.add_argument("--steps", type=int, default=1000, \
            help="nof training steps (default: %(default)s)")
    parser.add_argument("--output", type=Path, \
            help="output directory")
    parser.add_argument("--blocks", type=int, default=6, \
            help="nof blocks in the resnet (default: %(default)s)")
    parser.add_argument("--filters", type=int, default=64, \
            help="nof filters per convlayer in the resnet (default: %(default)s)")

    args = parser.parse_args()
    sys.exit(main(args))
