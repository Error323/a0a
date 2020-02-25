#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "azul/magics.h"
#include "azul/state.h"
#include "mcts/mcts.h"
#include "utils/random.h"
#include "version.h"

static const char kOutcome[] = {'D', 'W', 'L'};

void Stats(const int num_games) {
  State state;
  MoveList moves;
  int n;
  int m = 0;
  float b = 0;
  uint64_t a;
  std::vector<int> stats(kNumMoves, 0);

  for (int i = 0; i < num_games; i++) {
    while (!state.IsTerminal()) {
      int n = state.LegalMoves(moves);
      b += n;
      n = utils::Random::Get().GetInt(0, n - 1);
      a = std::hash<Move>()(moves[n]);
      stats[a]++;
      state.Step(moves[n]);
      m++;
    }
    state.Reset();
  }

  b /= m;
  int sum = 0;
  n = 0;
  for (int i = 0; i < kNumMoves; i++) {
    if (stats[i] > 0) {
      sum += stats[i];
      n++;
    }
  }
  VLOG(1) << "Branch factor: " << b << " Dirichlet alpha: " << ((sum / float(n)) / b);
}

struct Datapoint {
  Datapoint(const State &s, const Policy &pi, int z): s(s), pi(pi), z(z) {}
  void Serialize(std::ostream &stream) {
    stream << s.Serialize();
    stream.write(reinterpret_cast<char*>(pi.data()), pi.size()*sizeof(pi[0]));
    stream.write(reinterpret_cast<char*>(&z), sizeof(z));
    stream.flush();
  }
  State s;
  Policy pi;
  int8_t z;
};


void SaveGame(std::vector<Datapoint> game, State::Result result, const int num) {
  static std::string randstr = utils::Random::Get().GetString(8);
  std::stringstream ss;
  ss << "azul-" << num << "-" << randstr << ".bin";
  std::ofstream file(ss.str(), std::iostream::binary);

  for (int i = 0, n = game.size(); i < n; i++) {
    if (result == State::DRAW) {
      game[i].z = 0;
    } else {
      if ((game[i].s.Turn() + 1) == result) {
        game[i].z = 1;
      } else {
        game[i].z = -1;
      }
    }

    game[i].Serialize(file);
  }

  file.close();
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

  int N = 10000;

  MCTS mcts;
  Policy pi;
  float pbest;
  Move abest;

  std::vector<Datapoint> game;
  for (int i = 0; i < N; i++) {
    game.clear();
    state.Reset();
    mcts.Clear();
    int n = 0;

    while (!state.IsTerminal()) {
      pi = mcts.GetPolicy(state);
      game.emplace_back(Datapoint{state, pi, 0});
      pbest = std::numeric_limits<float>::lowest();
      for (int j = 0; j < kNumMoves; j++) {
        if (pi[j] > pbest) {
          pbest = pi[j];
          abest = Move(j);
        }
      }
      state.Step(abest);
      n++;
    }

    auto result = state.Winner();
    SaveGame(game, result, i);
    printf("%8i %03i %c\n", i, n + 1, kOutcome[result]);
  }

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}

