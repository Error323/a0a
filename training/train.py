#!/usr/bin/env python3

import sys
import random
import struct
import argparse
import multiprocessing
import tensorflow as tf
import numpy as np

from pathlib import Path
from tensorflow.keras.layers import Conv2D, Dense, BatchNormalization, AvgPool2D, Add, Activation, Flatten
from tensorflow.keras import Model, Input

BATCH_SIZE = 1024
NUM_PLANES = 49
IMG_SIZE = 5
NUM_MOVES = 180
STATE = "30s5sB10s10sIIBBBBb"
STATESIZE = struct.calcsize(STATE)
SAMPLESIZE = STATESIZE + NUM_MOVES*4 + 1 # size of a single datapoint (s, pi, z)
SKIP = 10 # randomly skip up to this many entries in a game

reg = tf.keras.regularizers.l2(l=1e-4)

def parse_function(data):
    """converts raw bytes containing (s, pi, z) to planes and tensors"""
    c, b, t, l1, l2, w1, w2, f1, f2, s1, s2, f = struct.unpack(STATE, data[:STATESIZE])
    pi = tf.io.decode_raw(data[STATESIZE:-1], tf.float32)
    z = tf.io.decode_raw(data[-1], tf.int8)
    z = tf.cast(z, tf.float32)
    
    return s, (pi, z)


def get_files(path):
    return files


def gen(filenames):
    for filename in filenames:
        with open(filename, "rb") as f:
            data = f.read()
        n = len(data) // SAMPLESIZE
        i = np.random.randint(0, SKIP)
        while i < n:
            yield data[i*SAMPLESIZE:i*SAMPLESIZE+SAMPLESIZE]
            i += np.random.randint(0, SKIP) + 1


def create_dataset(filenames, shuffle):
    num_cpus = max(multiprocessing.cpu_count() - 2, 1)
    ds = tf.data.Dataset.from_generator(gen, (tf.string), (tf.TensorShape([])), args=(filenames))
    ds = ds.map(parse_function, num_parallel_calls=num_cpus)
    if shuffle:
        ds = ds.shuffle(BATCH_SIZE * 64)
    ds = ds.batch(BATCH_SIZE)
    return ds


def conv(x, filters, kernel_size, strides=(1, 1)):
    x = Conv2D(filters, kernel_size, strides, padding='same', kernel_regularizer=reg, use_bias=False)(x)
    x = BatchNormalization(scale=False, beta_regularizer=reg)(x)
    x = Activation('relu')(x)
    return x


def res_block(inputs, filters, kernel_size):
    x = conv(inputs, filters, kernel_size)
    x = Conv2D(filters, kernel_size, (1, 1), padding='same', kernel_regularizer=reg, use_bias=False)(x)
    x = BatchNormalization(scale=False, beta_regularizer=reg)(x)
    x = Add()([x, inputs])
    x = Activation('relu')(x)
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
    v = Dense(64, kernel_regularizer=reg, bias_regularizer=reg, activation='relu')(v)
    v = Dense(1, kernel_regularizer=reg, bias_regularizer=reg, activation='tanh', name="value")(v)

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
            tf.summary.scalar('learning rate', data=lr, step=self.counter)
        self.counter += 1


def main(args):
    dst = args.output
    wgt = dst / "weights"
    logdir = str(dst / "logs/scalars")
    checkpoint_path = str(wgt)
    dst.mkdir(parents=True, exist_ok=True)

    file_writer = tf.summary.create_file_writer(logdir + "/metrics")
    file_writer.set_as_default()
    # Create a callback that saves the model's weights
    checkpoint_cb = tf.keras.callbacks.ModelCheckpoint(filepath=checkpoint_path, verbose=1)
    # Log various metrics on tensorboard with this callback
    tensorboard_cb = tf.keras.callbacks.TensorBoard(log_dir=logdir, update_freq=1000, profile_batch=0)

    inputs = Input(shape=(IMG_SIZE, IMG_SIZE, NUM_PLANES), name='planes')
    policy, value = resnet(inputs, 6, 64)
    model = Model(inputs, outputs=[policy, value])

    print(model.summary())

    losses = {"policy": "categorical_crossentropy", "value": "mean_squared_error"}

    # Compile and train
    model.compile(optimizer=tf.keras.optimizers.SGD(learning_rate=args.lr, momentum=0.9, nesterov=True),
            loss=losses,
            metrics=["accuracy"])

    """
    model.fit(train_ds, 
            validation_data=test_ds, 
            validation_steps=val_steps, 
            callbacks=[tensorboard_cb, learningrate_cb, checkpoint_cb, LogLr()])
    """

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AlphaZero Azul Trainer")
    parser.add_argument("--input", type=Path, \
            help="input directory")
    parser.add_argument("--resume", type=Path, default=None, \
            help="resume from this model")
    parser.add_argument("--lr", type=float, \
            help="learningrate to use this session")
    parser.add_argument("--steps", type=int, default=1000, \
            help="nof training steps (default: %(default)s)")
    parser.add_argument("--output", type=Path, \
            help="output directory")
    parser.add_argument("--blocks", type=int, default=6, \
            help="nof blocks in the resnet %(default)s")
    parser.add_argument("--filters", type=int, default=64, \
            help="nof filters per convlayer in the resnet %(default)s")

    args = parser.parse_args()
    sys.exit(main(args))
