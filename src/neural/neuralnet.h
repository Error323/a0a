#pragma once

#include <NvInferRuntime.h>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace nv = nvinfer1;
class Logger;

static constexpr int kNumBuffers = 3;

class NeuralNet {
 public:
  using NetBuffer = std::tuple<float*, float*, float*>;
  NeuralNet();
  ~NeuralNet();

  // Deserialize the tensorrt network from disk and construct engine
  void Load(const std::string &filename);

  // Obtains 3 buffers (input, policy, value)
  NetBuffer GetBuffers();

  // Indicate that the input is ready for this thread. This function is called
  // from multiple threads and will perform inference when the batch is filled.
  //
  // NOTE: Make sure to never spawn more threads than the max batch size
  void InputReady();

  // Reduces the batchsize (useful for when a thread is finished)
  void DecreaseBatchSize();

  int MaxBatchSize() { return max_batch_size_; }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  nv::IRuntime *runtime_;
  nv::ICudaEngine *engine_;
  nv::IExecutionContext *context_;
  std::unique_ptr<Logger> logger_;

  void *gpu_buffers_[kNumBuffers];
  float *host_buffers_[kNumBuffers];
  std::size_t sizes_[kNumBuffers];
  int max_batch_size_;
  int batch_size_;
  int soft_max_batch_size_;
  int buffer_index_;

  int input_id_;
  int policy_id_;
  int value_id_;

  // Performs actual forward inference on gpu, assumes batch is ready
  void Forward();
};
