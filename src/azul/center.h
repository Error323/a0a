#include <vector>
#include <stdint.h>

#include "constants.h"

class Center {
 public:
  /* part of an action or move */
  enum Position {FAC1, FAC2, FAC3, FAC4, FAC5, CENTER, NUM_POS};
  /* nof factories in 1v1 gameplay */
  static const int NUM_FACTORIES = 5;
  /* tiles a single factory can hold maximum */
  static const int NUM_TILES_PER_FACTORY = 4;
  /* nof tiles in the bag initially */
  static const int BAG_SIZE = 100;

  Center();

  void Reset();
  void NextRound();
  int Count(Position pos);
  int AddTile(Tile tile, Position pos, int num = 1);
  void TakeTiles(Position pos, Tile tile);

 private:
  std::vector<Tile> bag_;
  uint8_t bag_size_;
  // center encodes all factories and the center for each tile type
  uint64_t center_[NUM_TILES];
};
