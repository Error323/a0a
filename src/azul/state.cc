#include "state.h"

#include <glog/logging.h>

#include <sstream>

State::State() : center_(bag_), boards_{bag_, bag_} {
  Reset();
}

State::State(const State &s): center_(bag_), boards_{bag_, bag_} {
  *this = s;
}


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
    boards_[0].NextRound();
    boards_[1].NextRound();
    center_.NextRound();
    // It's theoretically possible to have no tiles in the center the entire
    // round. When each factory has 4 tiles of the same type.
    turn_ = center_.first == -1 ? 0 : center_.first;
  } else {
    turn_ ^= 1u;
  }
}

std::string State::Serialize() const {
  std::stringstream ss;
  uint8_t s;
  // turn 1 byte
  ss.write(reinterpret_cast<const char*>(&turn_), 1);

  // scores 2 bytes
  s = boards_[0].Score();
  ss.write(reinterpret_cast<char *>(&s), sizeof(s));
  s = boards_[1].Score();
  ss.write(reinterpret_cast<char *>(&s), sizeof(s));

  // bag 5 bytes
  ss.write(reinterpret_cast<const char *>(&bag_.tiles), sizeof(bag_.tiles));

  // factories + center 6*5 bytes
  ss.write(reinterpret_cast<const char *>(&center_.holders), sizeof(center_.holders));

  // first tile 1 byte
  ss.write(reinterpret_cast<const char *>(&center_.first), 1);

  for (int p = 0; p <= 1; p++) {
    // left 5*2 bytes
    ss.write(reinterpret_cast<const char *>(&boards_[p].left), sizeof(boards_[p].left));
    // floorline 1 byte
    ss.write(reinterpret_cast<const char *>(&boards_[p].floorline), 1);
    // wall 4 bytes
    ss.write(reinterpret_cast<const char *>(&boards_[p].wall), sizeof(boards_[p].wall));
  }

  return ss.str();
}

void State::MakePlanes(std::vector<float> &planes) {
  planes.resize(kNumPlanes * Board::SIZE * Board::SIZE);
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
    if (a > b) return PLAYER1;
    else return PLAYER2;
  } else {
    if (a > b) return PLAYER2;
    else return PLAYER1;
  }
}
