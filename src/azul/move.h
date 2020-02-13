#pragma once

#include <stdint.h>

#include "constants.h"

class Move {
 public:
  Move(uint8_t index);
  Move(Position factory, Tile tile, Line line);

  uint8_t index_;
  uint8_t factory_;
  uint8_t line_;
  Tile tile_type_;

 private:
  void Compose();
  void Decompose();
};
