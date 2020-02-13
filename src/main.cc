#include <glog/logging.h>

#include "azul/board.h"
#include "azul/magics.h"
#include "version.h"

int main(int argc, char **argv) {
  ::google::SetVersionString(VERSION);

  FLAGS_alsologtostderr = 1;
  FLAGS_v = 1;
  FLAGS_colorlogtostderr = 1;

  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  InitScoreTable();
  Board b;
  b.ApplyMove(Move(2), 3);
  b.NextRound();
  VLOG(1) << b.Score();

  ::google::ShutDownCommandLineFlags();
  ::google::ShutdownGoogleLogging();
  return 0;
}
