#include <glog/logging.h>
#include <vector>

#include "azul/magics.h"
#include "azul/state.h"
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
  State state;
  int depth = 6;
  std::vector<State> states(depth);

  for (int i = 0; i < depth; i++) {
    uint64_t nodes = perft(state, states, i);
    VLOG(1) << "perft(" << i << ") = " << nodes;
  }

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}
