#include "board.h"
#include <algorithm>

static const int PENALTY[] = {0, 1, 2, 4, 6, 8, 11, 14};

Board::Board() : wall_(0), score_(0), floor_line_(0) {}

void Board::Reset() {
  for (int i = 0; i < SIZE; i++) {
    left_[i].tile_type = left_[i].count = 0;
  }
  wall_ = 0;
  score_ = 0;
  floor_line_ = 0;
}

void Board::ApplyMove(Move move, int num_tiles) {
  // apply move to the "left" part
  if (move.line_ < SIZE) {
    left_[move.line_].tile_type = move.tile_type_;
    left_[move.line_].count += num_tiles;
    num_tiles = 0;
    // line is saturated
    if (left_[move.line_].count > move.line_ + 1) {
      num_tiles = left_[move.line_].count - (move.line_ + 1);
      left_[move.line_].count = move.line_ + 1;
    }
  }

  // put remaining tiles on the floorline
  floor_line_ += num_tiles;
}

void Board::IncreaseFloorline() { floor_line_++; }

static int Column(int row, int tile) { return (row + tile) % Board::SIZE; }

void Board::NextRound() {
  // 1. move left to the wall
  // 2. compute score
  // 3. determine if game over
  for (int i = 0; i < SIZE; i++) {
    if (left_[i].count == (i + 1)) {
      int j = Column(i, left_[i].tile_type);
      wall_ |= (1 << (i * SIZE + j));
      // NOTE: Use magic bitboards to determine row + column score
      // UpdateScore(i, j);
    }
  }

  // 4. update floor line
  score_ -= PENALTY[std::min(int(floor_line_), 7)];
  score_ = std::max(0, int(score_));
  floor_line_ = 0;
}

int16_t Board::Score() { return score_; }
