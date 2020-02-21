#include <glog/logging.h>

#include <fstream>
#include <vector>

#include "mcts/mcts.h"
#include "azul/magics.h"
#include "azul/state.h"
#include "utils/random.h"
#include "version.h"

uint64_t perft(State state, std::vector<State> &states, int depth) {
  if (depth == 0 || state.IsTerminal()) {
    return 1;
  }

  MoveList moves;
  int n = state.LegalMoves(moves);
  uint64_t nodes = 0;

  states[depth] = state;
  for (int i = 0; i < n; i++) {
    state.Step(moves[i]);
    nodes += perft(state, states, depth - 1);
    state = states[depth];
  }

  return nodes;
}

int main(int argc, char **argv) {
  ::google::SetVersionString(VERSION);

  FLAGS_alsologtostderr = 1;
  FLAGS_v = 1;
  FLAGS_colorlogtostderr = 1;

  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  InitScoreTable();
  MoveList moves;
  State state;

  int N = 10;
  MCTS mcts;
  Policy pi;
  float pbest;
  Move abest;

  for (int i = 0; i < N; i++) {
    state.Reset();
    mcts.Clear();

    while (!state.IsTerminal()) {
      pi = mcts.GetPolicy(state);
      pbest = std::numeric_limits<float>::lowest();
      for (int j = 0; j < kNumMoves; j++) {
        if (pi[j] > pbest) {
          pbest = pi[j];
          abest = Move(j);
        }
      }
      state.Step(abest);
    }
  }

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}
