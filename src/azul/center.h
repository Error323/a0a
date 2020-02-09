#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "constants.h"

class Bag {
 public:
  /* nof tiles in the bag initially */
  static const int BAG_SIZE = 100;

  Bag();
  void Reset();
  Tile Pop();

  uint8_t tiles[NUM_TILES];

 private:
  uint8_t size_;
};

class Holder {
 public:
  Holder();

  int Take(Tile tile);
  int Add(Tile tile, int num = 1);
  int Count();
  int Count(Tile tile);
  void Clear();

 private:
  static const int NBITS = 4;
  static const int MAX = (1 << NBITS) - 1;
  // 4 bits per type, total: 20 bit
  uint32_t counts_;
};

class Center {
 public:
  /* nof factories in 1v1 gameplay */
  static const int NUM_FACTORIES = 5;
  /* tiles a single factory can hold maximum */
  static const int NUM_TILES_PER_FACTORY = 4;
  /* maximum center tiles: 4*5 - 5 + 1 */
  static const int NUM_CENTER = (NUM_TILES_PER_FACTORY - 1) * NUM_FACTORIES;

  Center();

  std::string DebugStr();
  void CenterFromString(const std::string center);
  void Reset();
  bool IsRoundOver();
  void Clear();
  int Count(Position pos);
  int Count(Position pos, Tile tile);
  // assume we can always add tiles to the center legally
  void AddTile(Tile tile, Position pos, int num = 1);
  int TakeTiles(Position pos, Tile tile);

 private:
  Holder center_[NUM_POS];
  Bag bag_;
};
