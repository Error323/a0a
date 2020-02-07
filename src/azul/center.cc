#include <algorithm>

#include "center.h"

Center::Center() : bag_(BAG_SIZE), bag_size_(0) {
  for (int i = 0; i < NUM_TILES; i++) {
    for (int j = 0, n = BAG_SIZE / NUM_TILES; j < n; j++) {
      bag_[i * n + j] = Tile(i);
    }
  }

  NextRound();
}

void Center::Reset() {
  std::random_shuffle(bag_.begin(), bag_.end());
  bag_size_ = 100;
}

void Center::NextRound() {
  if (bag_size_ == 0) {
    Reset();
  }

  for (int i = 0; i < NUM_FACTORIES; i++) {
    for (int j = 0; j < NUM_TILES_PER_FACTORY; j++) {
      bag_size_--;
      Tile t = bag_[bag_size_];
      AddTile(t, Position(i));
    }
  }
}

int Center::Count(Position pos) {
  uint64_t f = center_[0];
  for (int i = 1; i < NUM_TILES; i++) {
    f |= center_[i];
  }
  f >>= pos * NUM_TILES_PER_FACTORY;
  return __builtin_popcount(f & 0xf);
}

int Center::AddTile(Tile tile, Position pos, int num) {
  int free = NUM_TILES_PER_FACTORY - Count(pos);
  int n = (1 << std::min(free, num)) - 1;
  center_[tile] |= n << (pos * NUM_TILES_PER_FACTORY);
  return num - free;
}

void Center::TakeTiles(Position pos, Tile tile) {
}
