#pragma once
#include <NvInferRuntime.h>
#include <utils/safequeue.h>

#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace nv = nvinfer1;
using output_t = std::pair<std::vector<float>, float>;

struct work_t {
  work_t() = default;
  std::vector<float> planes;
  std::shared_ptr<std::promise<output_t>> result;
};

class Gpu {
 public:
  explicit Gpu(int id, const std::vector<char> &net);
  ~Gpu();

  void AddWork(int i, work_t &&work);
  void Forward();
  [[nodiscard]] int MaxBatchSize() const { return max_batch_size_; }

  struct batch_t {
    void *gpu[3];
    void *cpu[3];
    cudaStream_t stream;
    std::vector<work_t> work;
  };

 private:
  std::vector<int> bindings_;
  int max_batch_size_;
  int batch_idx_{0};
  std::size_t sizes_[3];
  nv::IRuntime *runtime_;
  nv::ICudaEngine *engine_;
  nv::IExecutionContext *context_;
  std::vector<batch_t> batches_;
};

class GpuManager {
 public:
  GpuManager(const std::string &filename, utils::SafeQueue<work_t> &q);
  ~GpuManager();

 private:
  std::vector<std::thread> threads_;
  std::atomic_bool done_{false};
  utils::SafeQueue<work_t> &q_;

  void Run(int gpu_id, const std::vector<char> &net);
};
