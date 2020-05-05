#!/usr/bin/env python3

import sys
import argparse
import numpy as np
import tensorflow as tf

from pathlib import Path
from tensorflow.keras import Model, Input


def write_dense(f, w, name):
    f.write(f"{name} {w[0].shape} {w[1].shape}\n")
    for i in range(2):
        f.write(' '.join(str(e) for e in w[i].flatten()))
        f.write('\n')


def write_conv(f, w, name):
    f.write(f"{name} {w.shape}\n")
    f.write(' '.join(str(e) for e in w.flatten()))
    f.write('\n')


def write_bn(f, w, name):
    assert(w.shape[0] == 3)
    f.write(f"{name} {w.shape}\n")
    for i in range(3):
        f.write(' '.join(str(e) for e in w[i].flatten()))
        f.write('\n')


def main(args):
    output = args.input.parent / "weights.txt"
    model = tf.keras.models.load_model(str(args.input))

    with open(str(output), "wt") as f:
        # construct list of weights
        for layer in model.layers[1:]:
            w = layer.get_weights()
            w = np.array(w)
            if "conv" in layer.name:
                write_conv(f, w, layer.name)
                print(w.shape, layer.name)
            elif "batch_normalization" in layer.name:
                write_bn(f, w, layer.name)
                print(w.shape, layer.name)
            elif "dense" in layer.name:
                write_dense(f, w, layer.name)
                print(w[0].shape, w[1].shape, layer.name)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Weight exporter")
    parser.add_argument("input", type=Path, \
            help="export weights to textfile")

    args = parser.parse_args()
    sys.exit(main(args))
