#!/usr/bin/env python3

import sys
import argparse
import numpy as np
import tensorflow as tf

from pathlib import Path
from tensorflow.keras import Model, Input


def write_bn(iterator, f):
    for name in ["beta", "mean", "var"]:
        w = np.array(next(iterator))
        f.write(f"batchnorm:{name} {w.shape}\n")
        f.write(' '.join(str(e) for e in w.flatten()))
        f.write('\n')


def write_conv(w, f):
    f.write(f"conv2d {w.shape}\n")
    f.write(' '.join(str(e) for e in w.flatten()))
    f.write('\n')


def main(args):
    output = args.input.parent / "weights.txt"
    model = tf.keras.models.load_model(str(args.input))
    with open(str(output), "wt") as f:
        # construct list of weights from the base layer and head layers
        weight_list = model.layers[0].get_weights()
        for layer in model.layers[1:]:
            wl = layer.get_weights()
            for w in wl:
                weight_list.append(w)

        weight_list_iter = iter(weight_list)
        for wl in weight_list_iter:
            w = np.array(wl)
            print(w.shape)
            # fire module skip next 11 layers
            if w.ndim == 4 and w.shape[:2] == (1, 1) and w.shape[3] >= 16:
                write_conv(w, f)
                write_bn(weight_list_iter, f)
                c1x1 = np.array(next(weight_list_iter))
                write_conv(c1x1, f)
                c3x3 = np.array(next(weight_list_iter))
                write_bn(weight_list_iter, f)
                write_conv(c3x3, f)
                write_bn(weight_list_iter, f)
            # conv2d, skip next 4 layers
            else:
                write_conv(w, f)
                write_bn(weight_list_iter, f)

    print(f"written weights to {output} ({len(weight_list)} layers)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Kubiko weight exporter")
    parser.add_argument("input", type=Path, \
            help="export weights to textfile from this basemodel's weights path")

    args = parser.parse_args()
    sys.exit(main(args))
