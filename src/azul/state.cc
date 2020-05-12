#include "state.h"

#include <glog/logging.h>

#include <sstream>

State::State() : center_(bag_), boards_{bag_, bag_} { Reset(); }

State::State(const State &s) : center_(bag_), boards_{bag_, bag_} { *this = s; }

int State::LegalMoves(MoveList &moves) {
  int i = 0;
  Board &b = boards_[turn_];

  for (int pos = 0; pos < NUM_POS; pos++) {
    for (int tile = 0; tile < NUM_TILES; tile++) {
      int num = center_.Count(Position(pos), Tile(tile));
      if (num > 0) {
        for (int line = 0; line < NUM_LINES - 1; line++) {
          // if there's space on the left
          if (b.left[line].count < line + 1) {
            // if the left is empty or the tiles are of the current type
            if (b.left[line].tile_type == tile || b.left[line].count == 0) {
              // if the wall doesn't have the current tile yet
              if (!b.WallHasTile(Tile(tile), Line(line))) {
                Move &m = moves[i];
                m.factory = Position(pos);
                m.tile_type = Tile(tile);
                m.line = Line(line);
                i++;
              }
            }
          }
        }

        // we can always add tiles to the floorline
        Move &m = moves[i];
        m.factory = Position(pos);
        m.tile_type = Tile(tile);
        m.line = FLOORLINE;
        i++;
      }
    }
  }

  return i;
}

void State::FromString(const std::string center) {
  Reset();
  center_.CenterFromString(center);
}

void State::Reset() {
  turn_ = 0;
  bag_.Reset();
  center_.Reset();
  boards_[0].Reset();
  boards_[1].Reset();
}

void State::Step(const Move move) {
  int num = center_.TakeTiles(Position(move.factory), Tile(move.tile_type));
  boards_[turn_].ApplyMove(move, num);

  if (center_.first == -1 && move.factory == CENTER) {
    center_.first = turn_;
    boards_[turn_].IncreaseFloorline();
  }

  prev_turn_ = turn_;
  if (center_.IsRoundOver()) {
    // It's theoretically possible to have no tiles in the center the entire
    // round. When each factory has 4 tiles of the same type.
    turn_ = center_.first == -1 ? 0 : center_.first;
    boards_[0].NextRound();
    boards_[1].NextRound();
    center_.NextRound();
  } else {
    turn_ ^= 1u;
  }
}

std::string State::Serialize() const {
  // NOTE: Order matters here
  std::stringstream ss;
  uint8_t s;

  // factories + center 6*5 bytes
  ss.write(reinterpret_cast<const char *>(&center_.holders),
           sizeof(center_.holders));

  // bag 5 bytes
  ss.write(reinterpret_cast<const char *>(&bag_.tiles), sizeof(bag_.tiles));

  // turn 1 byte
  ss.write(reinterpret_cast<const char *>(&turn_), 1);

  // left 5*2 bytes p1 & p2
  ss.write(reinterpret_cast<const char *>(&boards_[0].left),
           sizeof(boards_[0].left));
  ss.write(reinterpret_cast<const char *>(&boards_[1].left),
           sizeof(boards_[1].left));

  // wall 4 bytes p1 & p2
  ss.write(reinterpret_cast<const char *>(&boards_[0].wall),
           sizeof(boards_[0].wall));
  ss.write(reinterpret_cast<const char *>(&boards_[1].wall),
           sizeof(boards_[1].wall));

  // floorline 1 byte p1 & p2
  ss.write(reinterpret_cast<const char *>(&boards_[0].floorline), 1);
  ss.write(reinterpret_cast<const char *>(&boards_[1].floorline), 1);

  // scores 2 bytes
  s = boards_[0].Score();
  ss.write(reinterpret_cast<char *>(&s), sizeof(s));
  s = boards_[1].Score();
  ss.write(reinterpret_cast<char *>(&s), sizeof(s));

  // first tile 1 byte
  ss.write(reinterpret_cast<const char *>(&center_.first), 1);

  auto str = ss.str();
  CHECK(str.size() == 69);
  return str;
}

void State::MakePlanes(float *planes) {
  // NOTE(Folkert): Order matters! Should be equal to training/generator.py

  // scores
  memset(planes, 0, sizeof(float) * kNumPlanes * Board::SIZE * Board::SIZE);
  int idx = 0;
  SetPlane(&planes[idx], boards_[turn_].Score() / 255.0f);
  idx++;
  SetPlane(&planes[idx], boards_[1 ^ turn_].Score() / 255.0f);
  idx++;

  // bag
  for (int t = 0; t < NUM_TILES; t++) {
    SetPlane(&planes[idx * 25], bag_.tiles[t] / 20.0f);
    idx++;
  }

  // factories + center
  for (int f = 0; f < NUM_POS; f++) {
    int empty = (f == CENTER ? 15 : 4);
    for (int t = 0; t < NUM_TILES; t++) {
      for (int j = 0; j < center_.Count(Position(f), Tile(t)); j++) {
        SetPlane(&planes[idx * 25], (t + 1) / 5.0f);
        empty--;
        idx++;
      }
    }
    for (int e = 0; e < empty; e++) idx++;
  }

  // first tile
  SetPlane(&planes[idx * 25], (center_.first + 1) / 2.0f);
  idx++;

  // me left
  for (int i = 0; i <= LINE5; i++) {
    for (int j = 0; j < boards_[turn_].left[i].count; j++) {
      planes[idx * 25 + i * 5 + j] =
          (boards_[turn_].left[i].tile_type + 1) / 5.0f;
    }
  }
  idx++;

  // me wall
  for (int i = 0; i < 25; i++)
    planes[idx * 25 + i] = (boards_[turn_].wall >> (24 - i)) & 1;
  idx++;

  // me floor
  SetPlane(&planes[idx * 25], boards_[turn_].floorline / 7.0f);
  idx++;

  // op left
  for (int i = 0; i <= LINE5; i++) {
    for (int j = 0; j < boards_[1 ^ turn_].left[i].count; j++) {
      planes[idx * 25 + i * 5 + j] =
          (boards_[1 ^ turn_].left[i].tile_type + 1) / 5.0f;
    }
  }
  idx++;

  // op wall
  for (int i = 0; i < 25; i++)
    planes[idx * 25 + i] = (boards_[1 ^ turn_].wall >> (24 - i)) & 1;
  idx++;

  // op floor
  SetPlane(&planes[idx * 25], boards_[1 ^ turn_].floorline / 7.0f);
  idx++;

  CHECK(idx == kNumPlanes) << "Invalid plane count " << idx
                           << " != " << kNumPlanes;
}

int State::Outcome() {
  int me = boards_[prev_turn_].Score();
  int op = boards_[1u ^ prev_turn_].Score();
  if (me > op) return 1;
  if (me < op) return -1;
  return 0;
}

bool State::IsTerminal() {
  return boards_[0].IsTerminal() || boards_[1].IsTerminal();
}

State &State::operator=(const State &s) {
  if (this == &s) {
    return *this;
  }

  bag_ = s.bag_;
  center_ = s.center_;
  boards_[0] = s.boards_[0];
  boards_[1] = s.boards_[1];
  turn_ = s.turn_;

  return *this;
}

State::Result State::Winner() {
  CHECK(IsTerminal());

  int a = boards_[prev_turn_].Score();
  int b = boards_[1u ^ prev_turn_].Score();

  if (a == b) return DRAW;

  if (prev_turn_ == 0) {
    if (a > b)
      return PLAYER1;
    else
      return PLAYER2;
  } else {
    if (a > b)
      return PLAYER2;
    else
      return PLAYER1;
  }
}

void State::SetPlane(float *plane, float v) {
  for (int i = 0; i < Board::SIZE * Board::SIZE; i++) plane[i] = v;
}
