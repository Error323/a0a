#include "board.h"
#include "magics.h"

#include <algorithm>

// masks for all tiles bonusses
static const uint32_t kAllTiles[] = {0x1041041, 0x182082, 0x20c104, 0x410608,
                                     0x820830};

// floorline penalties
static const int kPenalty[] = {0, 1, 2, 4, 6, 8, 11, 14};
static const int kFloorLineSize = 7;
static const int kBonusTiles = 10;
static const int kBonusCol = 7;
static const int kBonusRow = 2;

Board::Board(Bag &bag)
    : wall(0), score_(0), floor_line_(0), terminal_(false), bag_(&bag) {
  Reset();
}

void Board::Reset() {
  for (int i = 0; i < SIZE; i++) {
    left[i].tile_type = left[i].count = 0;
  }
  wall = 0;
  score_ = 0;
  floor_line_ = 0;
  terminal_ = false;
}

void Board::ApplyMove(Move move, int num_tiles) {
  // apply move to the "left" part
  if (move.line < SIZE) {
    left[move.line].tile_type = move.tile_type;
    left[move.line].count += num_tiles;
    num_tiles = 0;
    // line is saturated
    if (left[move.line].count > move.line + 1) {
      num_tiles = left[move.line].count - (move.line + 1);
      left[move.line].count = move.line + 1;
    }
  }

  // put remaining tiles on the floorline
  floor_line_ += num_tiles;
  // update bag statistics
  bag_->Return(Tile(move.tile_type), num_tiles);
}

void Board::IncreaseFloorline() { floor_line_++; }

void Board::UpdateScore(int row, int col, int tile) {
  // NOTE: Uses magic bitboards to determine row + column score
  score_ += GetScore(row * SIZE + col, wall);

  if ((wall & kRows[row]) == kRows[row]) {
    score_ += kBonusRow;
    terminal_ = true;
  }

  score_ += (wall & kColumns[col]) == kColumns[col] ? kBonusCol : 0;
  score_ += (wall & kAllTiles[tile]) == kAllTiles[tile] ? kBonusTiles : 0;
}

static uint32_t Column(int row, int tile) { return (row + tile) % Board::SIZE; }

bool Board::WallHasTile(Tile tile, Line line) {
  if (line == FLOORLINE) {
    return false;
  }

  uint32_t row = line;
  uint32_t col = Column(row, tile);
  return wall & (1ul << (row * SIZE + col));
}

void Board::NextRound() {
  // 1. move left to the wall and clear
  // 2. compute score
  // 3. determine if game over
  for (int i = 0; i < SIZE; i++) {
    if (left[i].count == (i + 1)) {
      int j = Column(i, left[i].tile_type);
      wall |= (1ul << (i * SIZE + j));
      UpdateScore(i, j, left[i].tile_type);

      // update bag statistics
      bag_->Return(Tile(left[i].tile_type), left[i].count - 1);
      left[i].count = 0;
    }
  }

  // 4. update floor line
  score_ -= kPenalty[std::min(int(floor_line_), kFloorLineSize)];
  score_ = std::max(0, int(score_));
  floor_line_ = 0;
}

int16_t Board::Score() { return score_; }
