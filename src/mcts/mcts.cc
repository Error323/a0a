#include "mcts.h"

#include <glog/logging.h>

#include <limits>

#include "azul/state.h"
#include "utils/random.h"

static const int kReserves = 1 << 17;

MCTS::MCTS() {
  Ns_.reserve(kReserves);
  Nsa_.reserve(kReserves);
  Qsa_.reserve(kReserves);
  Psa_.reserve(kReserves);
  Wsa_.reserve(kReserves);
}

Policy MCTS::GetPolicy(State& state, float temp, bool dirichlet) {
  MoveList moves;
  Policy pi;
  pi.fill(0.0f);
  int num_moves = state.LegalMoves(moves);
  for (int i = 0; i < simulations_; i++) {
    Search(state, 0, temp);
  }

  std::size_t s, a;
  s = std::hash<State>()(state);
  float sum = 0.0f, eta, p;
  constexpr float eps = 0.25f;
  for (int i = 0; i < num_moves; i++) {
    a = std::hash<Move>()(moves[i]);
    if (Nsa_.find(s ^ a) == Nsa_.end()) {
      continue;
    }

    if (dirichlet) {
      eta = utils::Random::Get().GetGamma(alpha_, 1.0);
      p = (1.0f - eps) * Nsa_[s ^ a] + eps * eta;
    } else {
      p = Nsa_[s ^ a];
    }
    pi[a] = p;
    sum += p;
  }

  for (auto&& x : pi) x /= sum;

  return pi;
}

float MCTS::Search(State& state, int depth, float temp) {
  if (state.IsTerminal()) return state.Outcome();

  MoveList moves;
  int n = state.LegalMoves(moves);
  std::size_t s = std::hash<State>()(state), a;
  float v;

  if (Ns_.find(s) == Ns_.end()) {
    v = utils::Random::Get().GetFloat(2.0f) - 1.0f;
    float sum = 0.0f, r;

    for (int i = 0; i < n; i++) {
      r = utils::Random::Get().GetFloat(1.0f);
      a = std::hash<Move>()(moves[i]);
      sum += r;
      Psa_[s ^ a] = r;
      Nsa_[s ^ a] = 0;
      Wsa_[s ^ a] = 0.0f;
      Qsa_[s ^ a] = 0.0f;
    }

    for (int i = 0; i < n; i++) {
      a = std::hash<Move>()(moves[i]);
      Psa_[s ^ a] /= sum;
    }

    Ns_[s] = 0;
    return v;
  }

  float ubest = std::numeric_limits<float>::lowest();
  Move abest;

  for (int i = 0; i < n; i++) {
    a = std::hash<Move>()(moves[i]);
    float nsa = depth < depth_ ? std::pow(Nsa_[s ^ a], 1.0f / temp) : Nsa_[s ^ a];
    float u = std::sqrt(Ns_[s]) / (1.0f + nsa);
    u *= cpuct_ * Psa_[s ^ a];
    u += Qsa_[s ^ a];
    if (u > ubest) {
      ubest = u;
      abest = moves[i];
    }
  }

  State state_prime = state;
  state_prime.Step(abest);
  v = Search(state_prime, depth + 1, temp);
  if (state.Turn() != state_prime.Turn()) v = -v;
  a = std::hash<Move>()(abest);
  Ns_[s]++;
  Nsa_[s ^ a]++;
  Wsa_[s ^ a] += v;
  Qsa_[s ^ a] = Wsa_[s ^ a] / Nsa_[s ^ a];

  return v;
}

void MCTS::Clear() {
  Ns_.clear();
  Nsa_.clear();
  Qsa_.clear();
  Psa_.clear();
  Wsa_.clear();
}
