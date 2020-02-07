#include "move.h"

Move::Move(std::uint8_t index): index_(index) {
  Compose();
}

Move::Move(std::uint8_t factory, Tile tile, std::uint8_t line):
  factory_(factory),
  tile_type_(tile),
  line_(line) {
  Decompose();
}

void Move::Compose() {
  index_ = factory_ * NUM_TILES * NUM_POS + tile_type_ * NUM_POS + line_;
}

void Move::Decompose() {
  factory_ = index_ / (NUM_TILES * NUM_POS);
  tile_type_ = Tile((index_ - factory_ * NUM_TILES * NUM_POS) / NUM_POS);
  line_ = index_ % NUM_POS;
}
