#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "azul/magics.h"
#include "azul/state.h"
#include "mcts/mcts.h"
#include "neural/neuralnet.h"
#include "utils/random.h"
#include "version.h"

DEFINE_int32(num_games, 10000, "Nof games to produce");
DEFINE_string(model, "network.trt.bin", "TensorRT Plan file");
DEFINE_string(output, ".", "Output directory to store games");

static const char kOutcome[] = {'D', 'W', 'L'};

struct Datapoint {
  Datapoint(const State &s, const Policy &pi, int z) : s(s), pi(pi), z(z) {}
  void Serialize(std::ostream &stream) {
    stream << s.Serialize();
    stream.write(reinterpret_cast<char *>(pi.data()),
                 pi.size() * sizeof(pi[0]));
    stream.write(reinterpret_cast<char *>(&z), sizeof(z));
    stream.flush();
  }
  State s;
  Policy pi;
  int8_t z;
};

std::string SaveGame(std::vector<Datapoint> game, State::Result result,
                     const int num) {
  thread_local std::string randstr = utils::Random::Get().GetString(8);
  std::stringstream ss;
  ss << FLAGS_output << "/azul-" << num << "-" << randstr << ".bin";
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
  return ss.str();
}

void SelfPlay(int num_games, NeuralNet &net) {
  MCTS mcts(net);
  Policy pi;
  Move abest;
  State state;

  std::vector<Datapoint> game;
  for (int i = 0; i < num_games; i++) {
    game.clear();
    state.Reset();
    mcts.Clear();
    int num_plies = 0;

    while (!state.IsTerminal()) {
      pi = mcts.GetPolicy(state, abest, 1e-5f, true);
      game.emplace_back(Datapoint{state, pi, 0});
      state.Step(abest);
      num_plies++;
    }

    auto result = state.Winner();
    auto filename = SaveGame(game, result, i);
    VLOG(1) << "[" << i + 1 << "/" << num_games << "] " << filename << " "
            << num_plies << " " << kOutcome[result];
  }

  net.DecreaseBatchSize();
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

  NeuralNet net;
  net.Load(FLAGS_model);
  int num_threads = net.MaxBatchSize();
  int num_games = FLAGS_num_games / num_threads;
  int remainder = FLAGS_num_games % num_threads;

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; i++) {
    std::thread t(SelfPlay, num_games + (remainder > 0), std::ref(net));
    remainder--;
    threads.push_back(std::move(t));
  }

  for (int i = 0; i < num_threads; i++) {
    threads[i].join();
  }

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}
