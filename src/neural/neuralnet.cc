#include "neuralnet.h"

#include <fstream>

#include "gpu_init.h"
#include "nnlogger.h"

NeuralNet::NeuralNet()
    : max_batch_size_(0),
      batch_size_(0),
      soft_max_batch_size_(0),
      buffer_index_(0) {
  logger_ = std::make_unique<Logger>();
  for (int i = 0; i < kNumBuffers; i++) {
    gpu_buffers_[i] = nullptr;
    host_buffers_[i] = nullptr;
    sizes_[i] = 0;
  }
}

NeuralNet::~NeuralNet() {
  for (int i = 0; i < kNumBuffers; i++) {
    cudaSafeCall(cudaFree(gpu_buffers_[i]));
    cudaSafeCall(cudaFreeHost(host_buffers_[i]));
  }
  context_->destroy();
  engine_->destroy();
  runtime_->destroy();
}

void NeuralNet::Load(const std::string &filename) {
  std::ifstream file(filename.c_str(), std::ios::binary);
  std::vector<char> data(std::istreambuf_iterator<char>(file), {});
  runtime_ = nv::createInferRuntime(*logger_);
  engine_ = runtime_->deserializeCudaEngine(data.data(), data.size());
  context_ = engine_->createExecutionContext();
  max_batch_size_ = engine_->getMaxBatchSize();
  soft_max_batch_size_ = max_batch_size_;
  batch_size_ = 0;

  // allocate buffers
  input_id_ = engine_->getBindingIndex("planes");
  policy_id_ = engine_->getBindingIndex("policy");
  value_id_ = engine_->getBindingIndex("value");
  sizes_[input_id_] = max_batch_size_ * 49 * 5 * 5 * sizeof(float);
  sizes_[policy_id_] = max_batch_size_ * 180 * sizeof(float);
  sizes_[value_id_] = max_batch_size_ * sizeof(float);

  cudaSafeCall(
      cudaMallocHost((void **)&host_buffers_[input_id_], sizes_[input_id_]));
  cudaSafeCall(
      cudaMallocHost((void **)&host_buffers_[policy_id_], sizes_[policy_id_]));
  cudaSafeCall(
      cudaMallocHost((void **)&host_buffers_[value_id_], sizes_[value_id_]));
  cudaSafeCall(cudaMalloc(&gpu_buffers_[input_id_], sizes_[input_id_]));
  cudaSafeCall(cudaMalloc(&gpu_buffers_[policy_id_], sizes_[policy_id_]));
  cudaSafeCall(cudaMalloc(&gpu_buffers_[value_id_], sizes_[value_id_]));
  cudaSafeCall(cudaDeviceSynchronize());

  std::vector<std::string> dt{"kFLOAT", "kHALF", "kINT8", "kINT32", "kBOOL"};
  VLOG(1) << "Loaded TensorRT Network " << filename << " (" << data.size()
          << " bytes)";
  VLOG(1) << "  Max Batch Size: " << max_batch_size_;
  VLOG(1) << "  Nof Layers: " << engine_->getNbLayers();
  VLOG(1) << "  Input DataType: " << dt[int(engine_->getBindingDataType(0))];
}

NeuralNet::NetBuffer NeuralNet::GetBuffers() {
  std::lock_guard<std::mutex> lock(mutex_);
  float *input = &host_buffers_[input_id_][buffer_index_ * 49 * 5 * 5];
  float *policy = &host_buffers_[policy_id_][buffer_index_ * 180];
  float *value = &host_buffers_[value_id_][buffer_index_];
  VLOG(1) << buffer_index_;
  buffer_index_++;
  return std::make_tuple(input, policy, value);
}

void NeuralNet::InputReady() {
  bool ready = false;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    batch_size_++;
    ready = batch_size_ == soft_max_batch_size_;
  }

  if (ready) {
    Forward();
    batch_size_ = 0;
    cv_.notify_all();
  } else {
    std::unique_lock lock(mutex_);
    cv_.wait(lock);
  }
}

void NeuralNet::DecreaseBatchSize() {
  bool ready = false;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    soft_max_batch_size_--;
    ready = batch_size_ == soft_max_batch_size_;
  }

  if (ready) {
    Forward();
    batch_size_ = 0;
    cv_.notify_all();
  }
}

void NeuralNet::Forward() {
  // copy data to gpu
  cudaSafeCall(cudaMemcpy(gpu_buffers_[input_id_], host_buffers_[input_id_],
                          sizes_[input_id_], cudaMemcpyHostToDevice));
  cudaSafeCall(cudaDeviceSynchronize());
  // forward inference
  context_->execute(max_batch_size_, gpu_buffers_);
  cudaSafeCall(cudaDeviceSynchronize());
  // copy back results
  cudaSafeCall(cudaMemcpy(host_buffers_[policy_id_], gpu_buffers_[policy_id_],
                          sizes_[policy_id_], cudaMemcpyDeviceToHost));
  cudaSafeCall(cudaMemcpy(host_buffers_[value_id_], gpu_buffers_[value_id_],
                          sizes_[value_id_], cudaMemcpyDeviceToHost));

  cudaSafeCall(cudaDeviceSynchronize());
}
