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
  // full bag reset, restores bag to full size (100)
  void Reset();

  // puts the returned tiles back into the bag
  void ReShuffle();

  // adds tiles to the return pile
  void Return(Tile tile, int num);

  // get a random tile from the bag
  Tile Pop();

  Bag &operator=(const Bag &bag);

  uint8_t tiles[NUM_TILES];

 private:
  uint8_t size_;
  uint8_t returned_[NUM_TILES];
};

class Holder {
 public:
  Holder();

  int Take(Tile tile);
  int Add(Tile tile, int num = 1);
  int Count();
  int Count(Tile tile);
  void Clear();
  Holder &operator=(const Holder &h);
  uint8_t counts_[NUM_TILES];
};

class Center {
 public:
  /* nof factories in 1v1 gameplay */
  static const int NUM_FACTORIES = 5;
  /* tiles a single factory can hold maximum */
  static const int NUM_TILES_PER_FACTORY = 4;
  /* maximum center tiles: 4*5 - 5 */
  static const int NUM_CENTER = (NUM_TILES_PER_FACTORY - 1) * NUM_FACTORIES;

  /* first tile belongs to {-1, 0, 1} (none, player 0, player 1) */
  int8_t first{-1};

  Center(Bag &bag);

  std::string DebugStr();
  void CenterFromString(const std::string center);
  void Reset();
  void NextRound();
  bool IsRoundOver();
  void Clear();
  int Count(Position pos);
  int Count(Position pos, Tile tile);
  // assume we can always add tiles to the center legally
  void AddTile(Tile tile, Position pos, int num = 1);
  int TakeTiles(Position pos, Tile tile);
  Center &operator=(const Center &c);
  Holder holders[NUM_POS];

 private:
  Bag &bag_;
};
