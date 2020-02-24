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


void SaveGame(std::vector<Datapoint> game, State::Result result, const int i) {
  static std::string randstr = utils::Random::Get().GetString(8);
  std::stringstream ss;
  ss << "azul-" << i << "-" << randstr << ".bin";
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

  int N = 50;
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

    SaveGame(game, state.Winner(), i);
    printf("%08i %03i %c\n", i, n, kOutcome[state.Winner()]);
  }

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}

