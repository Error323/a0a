#pragma once

#include <unordered_map>
#include "azul/move.h"

using Policy = std::array<float, kNumMoves>;

class State;

class MCTS {
 public:
  MCTS();

  Policy GetPolicy(State &state, float temp=1.0f);
  void Clear();


 private:
  std::unordered_map<std::size_t, int> Ns_;
  std::unordered_map<std::size_t, int> Nsa_;
  std::unordered_map<std::size_t, float> Qsa_;
  std::unordered_map<std::size_t, float> Psa_;
  std::unordered_map<std::size_t, float> Wsa_;

  static constexpr float cpuct_{1.2f};
  static constexpr int simulations_{800};

  float Search(State &state);
};