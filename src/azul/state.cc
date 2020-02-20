#include "state.h"

State::State() : center_(bag_), boards_{bag_, bag_} { Reset(); }

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
                m.Compose();
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
        m.Compose();
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

  if (center_.IsRoundOver()) {
    boards_[0].NextRound();
    boards_[1].NextRound();
    center_.NextRound();
    // It's theoretically possible to have no tiles in the center the entire
    // round. When each factory has 4 tiles of the same type.
    turn_ = center_.first == -1 ? 0 : center_.first;
  } else {
    turn_ ^= 1;
  }
}

std::vector<uint8_t> State::Serialize() {
  std::vector<uint8_t> data;
  return data;
}

void State::MakePlanes(std::vector<float> &planes) {
  // 1 + 1 + 5 + 5*4 + 16 + 1 + 1 + 1 + 1 + 1 + 1 + 1 = 50
  // |   |   |   |     |    |   |   |   |   |   |   |
  // |   |   |   |     |    |   |   |   |   |   |   them floor: v in {0,...,7}
  // |   |   |   |     |    |   |   |   |   |   them wall: v in {0, 1}
  // |   |   |   |     |    |   |   |   |   them left: v in {0,...,5}
  // |   |   |   |     |    |   |   |   us floor: v in {0,...,7}
  // |   |   |   |     |    |   |   us wall: v in {0, 1}
  // |   |   |   |     |    |   us left: v in {0,...,5}
  // |   |   |   |     |    1st tile: v in {-1, 0, 1}
  // |   |   |   |     center: v in {0,...,5}
  // |   |   |   factories: v in {0,...,5}
  // |   |   bag: v in {0,...,20}
  // |   them score: v in {0,...,255}
  // us score: v in {0,...,255}
  planes.resize(50 * Board::SIZE * Board::SIZE);
}

int State::Outcome() {
  // from the pov of the first player
  int p1 = boards_[0].Score();
  int p2 = boards_[1].Score();
  if (p1 > p2) return 1;
  if (p1 < p2) return -1;
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
  center_.SetBag(bag_);
  boards_[0] = s.boards_[0];
  boards_[0].SetBag(bag_);
  boards_[1] = s.boards_[1];
  boards_[1].SetBag(bag_);
  turn_ = s.turn_;

  return *this;
}
