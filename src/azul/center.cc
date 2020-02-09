#include <glog/logging.h>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>

#include "center.h"
#include "utils/random.h"

Bag::Bag() : size_(BAG_SIZE) { Reset(); }

void Bag::Reset() {
  for (int i = 0; i < NUM_TILES; i++) {
    tiles[i] = BAG_SIZE / NUM_TILES;
  }
  size_ = BAG_SIZE;
}

Holder::Holder() : counts_(0) {}

int Holder::Add(Tile tile, int num) {
  int current = Count(tile);
  num += current;
  CHECK(num <= MAX) << "num = " << num;
  uint32_t mask = MAX << (tile * NBITS);
  counts_ &= ~mask;
  counts_ |= num << (tile * NBITS);
  return num;
}

int Holder::Take(Tile tile) {
  uint32_t mask = MAX << (tile * NBITS);
  int num = Count(tile);
  counts_ &= ~mask;
  return num;
}

void Holder::Clear() { counts_ = 0; }

int Holder::Count(Tile tile) { return (counts_ >> (tile * NBITS)) & MAX; }

int Holder::Count() {
  int sum = 0;
  for (int i = 0; i < NUM_TILES; i++) {
    sum += Count(Tile(i));
  }
  return sum;
}

Tile Bag::Pop() {
  if (size_ == 0) {
    Reset();
  }

  float r = utils::Random::Get().GetFloat(1.0f);
  int sum = 0;
  // roulette wheel selection of a tile
  for (int i = 0; i < NUM_TILES - 1; i++) {
    sum += tiles[i];
    if (r < (sum / float(size_))) {
      tiles[i]--;
      size_--;
      return Tile(i);
    }
  }

  tiles[NUM_TILES - 1]--;
  size_--;
  return Tile(NUM_TILES - 1);
}

Center::Center() { Reset(); }

std::string Center::DebugStr() {
  std::stringstream ss;
  for (int i = 0; i < NUM_POS + 1; i++) {
    for (int j = 0; j < NUM_TILES; j++) {
      ss << center_[Position(i)].Count(Tile(j)) << " ";
    }
    ss << std::endl;
  }
  return ss.str();
}

void Center::CenterFromString(const std::string center) {
  CHECK(center.size() == NUM_FACTORIES * NUM_TILES_PER_FACTORY + NUM_CENTER);
  Clear();
  // fill the factories
  for (size_t i = 0; i < NUM_FACTORIES * NUM_TILES_PER_FACTORY; i++) {
    if (center[i] == '_') {
      continue;
    }
    Tile tile = Tile(center[i] - '0');
    Position pos = Position(i / NUM_TILES_PER_FACTORY);
    center_[pos].Add(tile);
  }

  // fill the center
  for (size_t i = 0; i < NUM_CENTER; i++) {
    size_t n = i + NUM_FACTORIES * NUM_TILES_PER_FACTORY;
    if (center[n] == '_') {
      continue;
    }
    Tile tile = Tile(center[n] - '0');
    center_[CENTER].Add(tile);
  }
}

void Center::Clear() {
  for (int i = 0; i < NUM_FACTORIES + 1; i++) {
    center_[i].Clear();
  }
}

void Center::Reset() {
  for (int i = 0; i < NUM_FACTORIES; i++) {
    for (int j = 0; j < NUM_TILES_PER_FACTORY; j++) {
      Tile t = bag_.Pop();
      center_[i].Add(t);
    }
  }
}

int Center::AddTile(Tile tile, Position pos, int num) {
  int count = center_[pos].Count();
  if (pos != CENTER) {
    int free = NUM_TILES_PER_FACTORY - count;
    if (num > free) {
      center_[pos].Add(tile, free);
      num -= free;
    } else {
      center_[pos].Add(tile, num);
      num = 0;
    }
  }

  center_[CENTER].Add(tile, num);
  return pos == CENTER ? 0 : num;
}

int Center::TakeTiles(Position pos, Tile tile) {
  int num = center_[pos].Take(tile);
  if (pos != CENTER) {
    for (int i = 0; i < NUM_TILES; i++) {
      if (i == tile) {
        continue;
      }
      int n = center_[pos].Take(Tile(i));
      center_[CENTER].Add(Tile(i), n);
    }
  }

  return num;
}

void Center::NextRound() {}

int Center::Count(Position pos, Tile tile) { return center_[pos].Count(tile); }

int Center::Count(Position pos) { return center_[pos].Count(); }

