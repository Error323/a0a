#include <algorithm>
#include <bitset>
#include <iostream>
#include <glog/logging.h>

#include "center.h"

Center::Center() : bag_(BAG_SIZE), bag_size_(0) {
  // fill up the bag with 20 tiles of each type
  for (int i = 0; i < NUM_TILES; i++) {
    for (int j = 0, n = BAG_SIZE / NUM_TILES; j < n; j++) {
      bag_[i * n + j] = Tile(i);
    }
  }

  NextRound();
}

void Center::BagFromString(const std::string bag) {
  CHECK(bag.size() == BAG_SIZE);

  for (int i = 0; i < Tile::NUM_TILES; i++) {
    center_[i] = 0ull;
  }

  for (size_t i = 0; i < bag.size(); i++) {
    Tile t = Tile(bag[i] - '0');
    bag_[BAG_SIZE - i - 1] = t;
  }

  bag_size_ = BAG_SIZE;
  NextRound();
}

void Center::CenterFromString(const std::string center) {
  CHECK(center.size() <= NUM_FACTORIES * NUM_TILES_PER_FACTORY + NUM_CENTER);
  Clear();
  for (size_t i = 0; i < center.size(); i++) {
    if (center[i] == '_') {
      continue;
    }

    Tile t = Tile(center[i] - '0');
    if (i < NUM_FACTORIES * NUM_TILES_PER_FACTORY) {
      AddTile(t, Position(i / NUM_TILES_PER_FACTORY));
    }
    else {
      AddTile(t, CENTER);
    }
  }
}

void Center::Clear() {
  Reset();
  for (int i = 0; i < Tile::NUM_TILES; i++) {
    center_[i] = 0ull;
  }
}

void Center::Reset() {
  std::random_shuffle(bag_.begin(), bag_.end());
  bag_size_ = BAG_SIZE;
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

int Center::Count(Position pos, Tile tile) {
  uint64_t f = center_[tile];
  f >>= pos * NUM_TILES_PER_FACTORY;
  uint64_t mask = pos == CENTER ? 0xffff : 0xf;
  return __builtin_popcountll(f & mask);
}

int Center::Count(Position pos) {
  uint64_t f = center_[0];
  for (int i = 1; i < NUM_TILES; i++) {
    f |= center_[i];
  }
  f >>= pos * NUM_TILES_PER_FACTORY;
  uint64_t mask = pos == CENTER ? 0xffff : 0xf;
  return __builtin_popcountll(f & mask);
}

int Center::AddTile(Tile tile, Position pos, int num) {
  uint64_t mask = pos == CENTER ? 0xffff : 0xf;
  mask <<= pos * NUM_TILES_PER_FACTORY;
  uint64_t f = center_[0];
  for (int i = 1; i < NUM_TILES; i++) {
    f |= center_[i];
  }
  uint64_t taken = 0ull;
  f = ~f & mask;
  while (f && num) {
    int i = __builtin_ffsll(f) - 1;
    taken |= (1ull << i);
    f ^= (1ull << i);
    num--;
  }
  center_[tile] |= taken;
  return num;
}

void Center::TakeTiles(Position pos, Tile tile) {
  (void) pos;
  (void) tile;
}
