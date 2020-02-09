#include "board.h"
#include <algorithm>

// masks for all tiles bonusses
static const uint32_t ALL_TILES[] = {0x1041041, 0x820830, 0x410608, 0x20c104,
                                     0x182082};
// masks for column bonus
static const uint32_t COLUMNS[] = {0x1084210, 0x842108, 0x421084, 0x210842,
                                   0x108421};
// masks for row bonus
static const uint32_t ROWS[] = {0x1f00000, 0xf8000, 0x7c00, 0x3e0, 0x1f};

// floorline penalties
static const int PENALTY[] = {0, 1, 2, 4, 6, 8, 11, 14};
static const int BONUS_TILES = 10;
static const int BONUS_COLS = 2;
static const int BONUS_ROWS = 7;

Board::Board() : wall_(0), score_(0), floor_line_(0), terminal_(false) {
  Reset();
}

void Board::Reset() {
  for (int i = 0; i < SIZE; i++) {
    left_[i].tile_type = left_[i].count = 0;
  }
  wall_ = 0;
  score_ = 0;
  floor_line_ = 0;
  terminal_ = false;
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

uint32_t Board::Lookup(int row, int col) {
  uint32_t mask = ROWS[row] | COLUMNS[col];
  mask &= wall_;
  return 0ul;
}

void Board::UpdateScore(int row, int col, int tile) {
  // NOTE: Use magic bitboards to determine row + column score
  score_ += __builtin_popcount(Lookup(row, col)) + 1;

  if ((wall_ & COLUMNS[col]) == COLUMNS[col]) {
    score_ += BONUS_COLS;
    terminal_ = true;
  }
  score_ += (wall_ & ROWS[row]) == ROWS[row] ? BONUS_ROWS : 0;
  score_ += (wall_ & ALL_TILES[tile]) == ALL_TILES[tile] ? BONUS_TILES : 0;
}

static int Column(int row, int tile) { return (row + tile) % Board::SIZE; }

void Board::NextRound() {
  // 1. move left to the wall
  // 2. compute score
  // 3. determine if game over
  for (int i = 0; i < SIZE; i++) {
    if (left_[i].count == (i + 1)) {
      int j = Column(i, left_[i].tile_type);
      wall_ |= (1 << (i * SIZE + j));
      UpdateScore(i, j, left_[i].tile_type);
    }
  }

  // 4. update floor line
  score_ -= PENALTY[std::min(int(floor_line_), 7)];
  score_ = std::max(0, int(score_));
  floor_line_ = 0;
}

int16_t Board::Score() { return score_; }
