#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>

#include "board.h"
#include "center.h"
#include "constants.h"
#include "move.h"

using MoveList = std::array<Move, kNumMoves>;

class State {
 public:
  friend struct std::hash<State>;

  State();
  int LegalMoves(MoveList &moves);
  void MakePlanes(std::vector<float> &planes);
  void Reset();
  void Step(const Move move);
  void FromString(const std::string center);
  State &operator=(const State &s);
  std::string Serialize() const;
  int Outcome();
  bool IsTerminal();

 private:
  Bag bag_;
  Center center_;
  std::array<Board, 2> boards_;
  uint8_t turn_{0};
};

namespace std {
template<>
struct hash<State> {
  std::size_t operator()(const State &state) const {
    return std::hash<std::string>()(state.Serialize());
  }
};
}