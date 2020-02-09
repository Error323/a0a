#pragma once

#include <stdint.h>

#include "constants.h"
#include "move.h"

class Board {
 public:
  static const int SIZE = 5;

  Board();

  // always assumes a legal move
  void ApplyMove(Move move, int num_tiles);
  void IncreaseFloorline();
  void NextRound();
  void Reset();
  int16_t Score();

 private:
  struct Line {
    uint8_t tile_type;
    uint8_t count;
  };

  Line left_[SIZE];
  uint32_t wall_;  ///< using 25 bit for the wall (1 bit per tile)
  int16_t score_;
  uint8_t floor_line_;
};
