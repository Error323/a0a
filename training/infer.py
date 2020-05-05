#!/usr/bin/env python3

import sys
import argparse
import numpy as np
import tensorflow as tf

from pathlib import Path
from tensorflow.keras import Model, Input


def main(args):
    model = tf.keras.models.load_model(str(args.input))
    planes = np.zeros((1, 5, 5, 49), dtype=np.float32)
    pi, v = model.predict(planes)
    print(pi.shape, v.shape)
    print("pi: ", end='')
    for i,x in enumerate(pi.flatten()):
        if x > 0:
            print(f"{i}:{x:0.3f} ", end='')
    print(f"\nv: {v.flatten()[0]:0.3f}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Weight exporter")
    parser.add_argument("input", type=Path, \
            help="export weights to textfile")

    args = parser.parse_args()
    sys.exit(main(args))
