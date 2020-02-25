#include "center.h"
#include "utils/random.h"

#include <glog/logging.h>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <sstream>

// ============================================================================
// Bag class
// ============================================================================
Bag::Bag() : size_(0) { Reset(); }

void Bag::Reset() {
  for (int i = 0; i < NUM_TILES; i++) {
    tiles[i] = BAG_SIZE / NUM_TILES;
    returned_[i] = 0;
  }
  size_ = BAG_SIZE;
}

Tile Bag::Pop() {
  if (size_ == 0) {
    ReShuffle();
  }

  int r = utils::Random::Get().GetInt(0, size_ - 1);
  int sum = 0;
  // roulette wheel selection of a tile
  for (int i = 0; i < NUM_TILES - 1; i++) {
    sum += tiles[i];
    if (r < sum) {
      tiles[i]--;
      size_--;
      return Tile(i);
    }
  }

  tiles[NUM_TILES - 1]--;
  size_--;
  return Tile(NUM_TILES - 1);
}

void Bag::ReShuffle() {
  size_ = 0;
  for (int i = 0; i < NUM_TILES; i++) {
    tiles[i] += returned_[i];
    size_ += tiles[i];
    returned_[i] = 0;
  }
}

void Bag::Return(Tile tile, int num) { returned_[tile] += num; }

Bag& Bag::operator=(const Bag& bag) {
  if (this == &bag) {
    return *this;
  }

  size_ = bag.size_;
  for (int i = 0; i < NUM_TILES; i++) {
    tiles[i] = bag.tiles[i];
    returned_[i] = bag.returned_[i];
  }

  return *this;
}

// ============================================================================
// Holder class
// ============================================================================
Holder::Holder() { Clear(); }

int Holder::Add(Tile tile, int num) {
  counts_[tile] += num;
  return counts_[tile];
}

int Holder::Take(Tile tile) {
  int num = Count(tile);
  counts_[tile] = 0;
  return num;
}

void Holder::Clear() { for (int i = 0; i < NUM_TILES; i++) counts_[i] = 0; }

int Holder::Count(Tile tile) { return counts_[tile]; }

int Holder::Count() {
  int sum = 0;
  for (int i = 0; i < NUM_TILES; i++) {
    sum += counts_[i];
  }
  return sum;
}

Holder& Holder::operator=(const Holder& h) {
  if (this == &h) {
    return *this;
  }

  for (int i = 0; i < NUM_TILES; i++) counts_[i] = h.counts_[i];

  return *this;
}

// ============================================================================
// Center class
// ============================================================================
Center::Center(Bag &bag) : bag_(bag) { Reset(); }

std::string Center::DebugStr() {
  std::stringstream ss;
  for (int i = 0; i < NUM_POS; i++) {
    for (int j = 0; j < NUM_TILES; j++) {
      ss << holders[Position(i)].Count(Tile(j)) << " ";
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
    holders[pos].Add(tile);
  }

  // fill the center
  for (size_t i = 0; i < NUM_CENTER; i++) {
    size_t n = i + NUM_FACTORIES * NUM_TILES_PER_FACTORY;
    if (center[n] == '_') {
      continue;
    }
    Tile tile = Tile(center[n] - '0');
    holders[CENTER].Add(tile);
  }
}

void Center::Clear() {
  for (int i = 0; i < NUM_FACTORIES + 1; i++) {
    holders[i].Clear();
  }
  first = -1;
}

void Center::Reset() {
  first = -1;
  for (int i = 0; i < NUM_FACTORIES; i++) {
    holders[i].Clear();
    for (int j = 0; j < NUM_TILES_PER_FACTORY; j++) {
      Tile t = bag_.Pop();
      holders[i].Add(t);
    }
  }
}

void Center::AddTile(Tile tile, Position pos, int num) {
  holders[pos].Add(tile, num);
}

int Center::TakeTiles(Position pos, Tile tile) {
  int num = holders[pos].Take(tile);
  if (pos != CENTER) {
    for (int i = 0; i < NUM_TILES; i++) {
      if (i == tile) {
        continue;
      }
      // move remaining tiles from the factory to the center
      int n = holders[pos].Take(Tile(i));
      holders[CENTER].Add(Tile(i), n);
    }
  }

  return num;
}

bool Center::IsRoundOver() {
  for (int i = 0; i < NUM_POS; i++) {
    if (holders[i].Count() > 0) {
      return false;
    }
  }
  return true;
}

int Center::Count(Position pos, Tile tile) { return holders[pos].Count(tile); }

int Center::Count(Position pos) { return holders[pos].Count(); }

void Center::NextRound() {
  for (int i = 0; i < NUM_FACTORIES; i++) {
    for (int j = 0; j < NUM_TILES_PER_FACTORY; j++) {
      Tile t = bag_.Pop();
      holders[i].Add(t);
    }
  }
  first = -1;
}

Center& Center::operator=(const Center& c) {
  if (this == &c) {
    return *this;
  }

  for (int i = 0; i < NUM_POS; i++) holders[i] = c.holders[i];
  first = c.first;

  return *this;
}
