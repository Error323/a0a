#pragma once

#include <stdint.h>

#include "center.h"
#include "constants.h"
#include "move.h"

class Board {
 public:
  static const int SIZE = 5;

  // masks for columns
  static constexpr uint32_t kColumns[] = {0x108421, 0x210842, 0x421084,
                                          0x842108, 0x1084210};
  // masks for rows
  static constexpr uint32_t kRows[] = {0x1f, 0x3e0, 0x7c00, 0xf8000, 0x1f00000};

  Board(Bag &bag);

  // always assumes a legal move
  void ApplyMove(Move move, int num_tiles);
  void IncreaseFloorline();
  void NextRound();
  void Reset();
  int16_t Score();
  bool IsTerminal() { return terminal_; }
  bool WallHasTile(Tile tile, Line line);
  struct LLine {
    uint8_t tile_type : 4;
    uint8_t count : 4;
  };
  LLine left[SIZE];
  uint32_t wall;  ///< using 25 bit for the wall (1 bit per tile)

 private:
  int16_t score_;
  uint8_t floor_line_;
  bool terminal_;
  Bag &bag_;

  void UpdateScore(int row, int col, int tile);
};
