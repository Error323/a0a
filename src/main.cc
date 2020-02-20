#include <glog/logging.h>

#include <vector>

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

  float avg_steps = 0.0f;
  float avg_branch = 0.0f;
  int N = 100000;
  int outcome[3] = {0};
  int stepdist[200] = {0};

  for (int i = 0; i < N; i++) {
    state.Reset();
    int n, steps = 0;
    while (!state.IsTerminal()) {
      n = state.LegalMoves(moves);
      avg_branch += n;
      n = utils::Random::Get().GetInt(0, n - 1);
      state.Step(moves[n]);
      steps++;
    }
    stepdist[steps]++;
    outcome[state.Outcome() + 1]++;
    avg_steps += steps;
  }

  VLOG(1) << "Average steps: " << (avg_steps / N) << ", branch factor: " << (avg_branch / avg_steps);
  VLOG(1) << "W/D/L " << outcome[2] << "/" << outcome[1] << "/" << outcome[0];
  for (int i = 0; i < 200; i++) {
    VLOG(1) << i << " " << stepdist[i];
  }

/*  int depth = 6;
  std::vector<State> states(depth);

  for (int i = 0; i < depth; i++) {
    uint64_t nodes = perft(state, states, i);
    VLOG(1) << "perft(" << i << ") = " << nodes;
  }*/

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}
