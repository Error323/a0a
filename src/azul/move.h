#pragma once

#include <stdint.h>
#include <unordered_map>

#include "constants.h"

class Move {
 public:
  Move() : factory(FAC1), line(LINE1), tile_type(BLUE) {}
  Move(uint8_t id);
  Move(Position factory, Tile tile, Line line);

  Position factory;
  Line line;
  Tile tile_type;

 private:
  void Decompose(int id);
};

namespace std {
template<>
struct hash<Move> {
  std::size_t operator()(const Move &move) const {
    return move.factory * NUM_TILES * NUM_POS + move.tile_type * NUM_POS + move.line;
  }
};
}