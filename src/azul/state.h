#pragma once

#include <stdint.h>

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "board.h"
#include "center.h"
#include "constants.h"
#include "move.h"

class State {
 public:
  enum Result { DRAW, PLAYER1, PLAYER2 };
  friend struct std::hash<State>;

  State();
  State(const State &s);
  int LegalMoves(MoveList &moves);
  Result Winner();
  int Turn() { return turn_; }
  void MakePlanes(float *planes);
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
  uint8_t prev_turn_{0};
  void SetPlane(float *plane, float v);
};

namespace std {
template <>
struct hash<State> {
  std::size_t operator()(const State &state) const {
    return std::hash<std::string>()(state.Serialize());
  }
};
}  // namespace std