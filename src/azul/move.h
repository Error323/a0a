#pragma once

#include <stdint.h>

#include "constants.h"

class Move {
 public:
  Move() : id(0), factory(0), line(0), tile_type(BLUE) {}
  Move(uint8_t index);
  Move(Position factory, Tile tile, Line line);

  uint8_t id;
  uint8_t factory;
  uint8_t line;
  Tile tile_type;

 private:
  void Compose();
  void Decompose();
};
