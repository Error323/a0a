#include "gpumanager.h"

#include <glog/logging.h>
#include <utils/safequeue.h>
#include <iostream>

static constexpr int kNumThreads = 128;

void Run(int tid, utils::SafeQueue<work_t> &q) {
  std::vector<output_t> v;
  for (int i = 0; i < 800; i++) {
    work_t work;
    work.planes = std::vector<float>(49*5*5, tid / float(kNumThreads));
    work.result = std::make_shared<std::promise<output_t>>();
    q.Enqueue(work);
    auto x = work.result->get_future().get();
    v.emplace_back(x);
  }
  VLOG(1) << v.size() << v[1].second;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    LOG(FATAL) << "no weights file given";
  }

  FLAGS_alsologtostderr = 1;
  FLAGS_v = 2;
  FLAGS_colorlogtostderr = 1;
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  utils::SafeQueue<work_t> q;
  GpuManager gm(argv[1], q);

  std::vector<std::thread> threads;
  for (int i = 0; i < kNumThreads; i++) {
    threads.emplace_back(std::thread(Run, i, std::ref(q)));
  }

  for (auto &t : threads) {
    t.join();
  }

  ::google::ShutdownGoogleLogging();

  return 0;
}
