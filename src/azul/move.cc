#include "move.h"
#include <iostream>

Move::Move(uint8_t index) : index_(index) { Decompose(); }

Move::Move(Position factory, Tile tile, Line line)
    : factory_(factory), line_(line), tile_type_(tile) {
  Compose();
}

void Move::Compose() {
  index_ = factory_ * NUM_TILES * NUM_POS + tile_type_ * NUM_POS + line_;
}

void Move::Decompose() {
  factory_ = index_ / (NUM_TILES * NUM_POS);
  tile_type_ = Tile((index_ - factory_ * NUM_TILES * NUM_POS) / NUM_POS);
  line_ = index_ % NUM_POS;
}
