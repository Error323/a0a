#include "move.h"

Move::Move(uint8_t id) { Decompose(id); }

Move::Move(Position factory, Tile tile, Line line)
    : factory(factory), line(line), tile_type(tile) {
}

void Move::Decompose(int id) {
  factory = Position(id / (NUM_TILES * NUM_POS));
  tile_type = Tile((id - factory * NUM_TILES * NUM_POS) / NUM_POS);
  line = Line(id % NUM_POS);
}
