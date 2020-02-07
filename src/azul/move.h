#include "constants.h"

class Move {
public:
  Move(std::uint8_t index);
  Move(std::uint8_t factory, Tile tile, std::uint8_t line);

  std::uint8_t index_;
  std::uint8_t factory_;
  std::uint8_t line_;
  Tile tile_type_;

private:
  void Compose();
  void Decompose();
};
