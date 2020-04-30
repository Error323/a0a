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

reg = tf.keras.regularizers.l2(l=1e-4)


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
    def __init__(self, steps=100):
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
    logdir = str(dst / "logs/scalars")
    model_path = dst / f"model-{args.blocks}x{args.filters}"
    dst.mkdir(parents=True, exist_ok=True)


    train_ds = Generator(args.input / "train", True, BATCH_SIZE)
    test_ds = Generator(args.input / "test", False, BATCH_SIZE)

    file_writer = tf.summary.create_file_writer(logdir + "/metrics")
    file_writer.set_as_default()
    # Log various metrics on tensorboard with this callback
    tensorboard_cb = tf.keras.callbacks.TensorBoard(log_dir=logdir, update_freq=100, profile_batch=0)

    if model_path.exists():
        print(f"Loading model from {model_path}")
        model = tf.keras.models.load_model(str(model_path))
    else:
        inputs = Input(shape=(IMG_SIZE, IMG_SIZE, NUM_PLANES), name="planes")
        policy, value = resnet(inputs, args.blocks, args.filters)
        model = Model(inputs, outputs=[policy, value], name=f"ResNet-{args.blocks}x{args.filters}")
        losses = {"policy": "categorical_crossentropy", "value": "mean_squared_error"}

        print(model.summary())

        model.compile(optimizer=tf.keras.optimizers.SGD(learning_rate=args.lr, momentum=0.9, nesterov=True),
                loss=losses,
                metrics=["accuracy"])

    model.fit(x=train_ds, 
            steps_per_epoch=args.steps,
            validation_data=test_ds, 
            callbacks=[tensorboard_cb, LogLr()],
            workers=4,
            use_multiprocessing=True)

    model.save(str(model_path))

    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AlphaZero Azul Trainer")
    parser.add_argument("--input", type=Path, \
            help="input directory containing test/ and train/ subdirs")
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
