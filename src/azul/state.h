#pragma once

#include <stdint.h>
#include <array>
#include <string>

#include "board.h"
#include "center.h"
#include "constants.h"
#include "move.h"

using MoveList = std::array<Move, kNumMoves>;

class State {
 public:
  State();
  int LegalMoves(MoveList &moves);
  void MakePlanes(std::vector<float> &planes);
  void Reset();
  void Step(const Move move);
  void FromString(const std::string center);
  State &operator=(const State &s);
  std::string Serialize();
  int Outcome();
  bool IsTerminal();

 private:
  Bag bag_;
  Center center_;
  std::array<Board, 2> boards_;
  int turn_{0};
};
