#include "move.h"
#include <iostream>

Move::Move(uint8_t index) : id(index) { Decompose(); }

Move::Move(Position factory, Tile tile, Line line)
    : factory(factory), line(line), tile_type(tile) {
  Compose();
}

void Move::Compose() {
  id = factory * NUM_TILES * NUM_POS + tile_type * NUM_POS + line;
}

void Move::Decompose() {
  factory = id / (NUM_TILES * NUM_POS);
  tile_type = Tile((id - factory * NUM_TILES * NUM_POS) / NUM_POS);
  line = id % NUM_POS;
}
