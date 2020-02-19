#pragma once

#include <stdint.h>

#include "constants.h"

class Move {
 public:
  Move() : id(0), factory(FAC1), line(LINE1), tile_type(BLUE) {}
  Move(uint8_t index);
  Move(Position factory, Tile tile, Line line);
  void Compose();

  uint8_t id;
  Position factory;
  Line line;
  Tile tile_type;

 private:
  void Decompose();
};

static_assert(sizeof(Move) == 4);