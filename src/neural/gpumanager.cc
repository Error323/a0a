#include "gpumanager.h"

#include <NvInferRuntime.h>
#include <glog/logging.h>

#include <fstream>

#include "gpu_init.h"
#include "nnlogger.h"

static constexpr int kStreams = 8;

GpuManager::GpuManager(const std::string &filename, utils::SafeQueue<work_t> &q)
    : q_(q) {
  std::ifstream file(filename, std::ios::binary);
  std::vector<char> data(std::istreambuf_iterator<char>(file), {});
  int count;
  cudaSafeCall(cudaGetDeviceCount(&count));
  VLOG(1) << "Load `" << filename << "' (" << data.size() << " bytes)";
  VLOG(1) << "Initializing " << count << " gpus";
  for (int i = 0; i < count; i++) {
    threads_.emplace_back(std::thread(&GpuManager::Run, this, i, data));
  }
}

GpuManager::~GpuManager() {
  done_ = true;
  for (auto &t : threads_) {
    t.join();
  }
}

void GpuManager::Run(int gpu_id, const std::vector<char> &net) {
  Gpu gpu(gpu_id, net);

  while (!done_) {
    for (int i = 0; i < gpu.MaxBatchSize(); i++) {
      try {
        gpu.AddWork(i, q_.Dequeue());
      } catch (std::runtime_error) {
        return;
      }
    }
    gpu.Forward();
  }
}

Gpu::Gpu(int id, const std::vector<char> &net) {
  static Logger logger;
  cudaSafeCall(cudaSetDevice(id));
  cudaSafeCall(cudaDeviceReset());
  runtime_ = nv::createInferRuntime(logger);
  engine_ = runtime_->deserializeCudaEngine(net.data(), net.size());
  context_ = engine_->createExecutionContext();
  max_batch_size_ = engine_->getMaxBatchSize();
  VLOG(1) << "gpu(" << id << ") has batchsize: " << max_batch_size_;
  bindings_.emplace_back(engine_->getBindingIndex("planes"));
  bindings_.emplace_back(engine_->getBindingIndex("policy"));
  bindings_.emplace_back(engine_->getBindingIndex("value"));
  sizes_[bindings_[0]] = 49 * 5 * 5;
  sizes_[bindings_[1]] = 180;
  sizes_[bindings_[2]] = 1;

  for (int i = 0; i < kStreams; i++) {
    batch_t batch;
    cudaSafeCall(cudaStreamCreate(&batch.stream));
    for (auto b : bindings_) {
      cudaSafeCall(cudaMalloc(&batch.gpu[b],
                              sizes_[b] * max_batch_size_ * sizeof(float)));
      cudaSafeCall(cudaMallocHost(&batch.cpu[b],
                                  sizes_[b] * max_batch_size_ * sizeof(float)));
    }
    batch.work.resize(max_batch_size_);
    batches_.emplace_back(batch);
  }
  cudaSafeCall(cudaDeviceSynchronize());
}

Gpu::~Gpu() {
  for (auto &batch : batches_) {
    for (auto b : bindings_) {
      cudaSafeCall(cudaFree(batch.gpu[b]));
      cudaSafeCall(cudaFreeHost(batch.cpu[b]));
    }
    cudaSafeCall(cudaStreamDestroy(batch.stream));
  }
  context_->destroy();
  engine_->destroy();
  runtime_->destroy();
  cudaSafeCall(cudaDeviceReset());
}

void Gpu::AddWork(int i, work_t &&work) { batches_[batch_idx_].work[i] = std::move(work); }

void callback(void *context) {
  auto batch = reinterpret_cast<Gpu::batch_t *>(context);
  for (int i = 0, n = batch->work.size(); i < n; i++) {
    auto ptr = reinterpret_cast<float *>(batch->cpu[1]) + i * 180;
    std::vector<float> pi(180);
    std::copy(ptr, ptr + 180, pi.begin());
    float v = reinterpret_cast<float *>(batch->cpu[2])[i];
    batch->work[i].result->set_value(std::make_pair(std::move(pi), v));
  }
}

void Gpu::Forward() {
  auto &batch = batches_[batch_idx_];

  auto input = bindings_[0];
  auto out1 = bindings_[1];
  auto out2 = bindings_[2];

  // copy memory into host buffer
  for (int i = 0; i < max_batch_size_; i++) {
    auto dst = reinterpret_cast<float *>(batch.cpu[input]) + i * sizes_[input];
    auto src = batch.work[i].planes.data();
    auto size = sizes_[input] * sizeof(float);
    memcpy(dst, src, size);
  }

  // copy input batch to device memory
  cudaSafeCall(cudaMemcpyAsync(batch.gpu[input], batch.cpu[input],
                               sizes_[input] * max_batch_size_ * sizeof(float),
                               cudaMemcpyHostToDevice, batch.stream));

  // perform forward inference
  context_->enqueue(max_batch_size_, batch.gpu, batch.stream, nullptr);

  // copy output back to host buffers
  cudaSafeCall(cudaMemcpyAsync(batch.cpu[out1], batch.gpu[out1],
                               sizes_[out1] * max_batch_size_ * sizeof(float),
                               cudaMemcpyDeviceToHost, batch.stream));
  cudaSafeCall(cudaMemcpyAsync(batch.cpu[out2], batch.gpu[out2],
                               sizes_[out2] * max_batch_size_ * sizeof(float),
                               cudaMemcpyDeviceToHost, batch.stream));

  // attach callback that notifies promises
  cudaSafeCall(cudaLaunchHostFunc(batch.stream, callback, (void *)&batch));
  batch_idx_++;
  batch_idx_ %= kStreams;

  // ensure the next stream is ready for action
  cudaSafeCall(cudaStreamSynchronize(batches_[batch_idx_].stream));
}
