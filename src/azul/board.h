#include "constants.h"

class Board {
public:
  Board();
  void ApplyMove(Move move, int num_tiles);
  void SetFirst();

private:
  std::bitset<SIZE * SIZE> wall_;
  std::bitset<SIZE> left_[NUM_TILES];
  std::uint8_t floor_line_;
};
