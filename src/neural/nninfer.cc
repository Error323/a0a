#include "gpu_init.h"

#include <NvInferRuntime.h>
#include <glog/logging.h>

#include <fstream>
#include <iostream>

namespace nv = nvinfer1;

class Logger : public nv::ILogger {
 public:
  void log(Severity severity, const char* msg) override {
    // log all the things
    switch (severity) {
      case nv::ILogger::Severity::kVERBOSE:
        VLOG(2) << msg;
        break;
      case nv::ILogger::Severity::kWARNING:
        LOG(WARNING) << msg;
        break;
      case nv::ILogger::Severity::kERROR:
        LOG(ERROR) << msg;
        break;
      case nv::ILogger::Severity::kINTERNAL_ERROR:
        LOG(FATAL) << msg;
        break;
      default:
        LOG(INFO) << msg;
        break;
    }
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    LOG(FATAL) << "no weights file given";
  }

  FLAGS_alsologtostderr = 1;
  FLAGS_v = 2;
  FLAGS_colorlogtostderr = 1;
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  static Logger logger;
  std::ifstream file(argv[1], std::ios::binary);
  std::vector<char> data(std::istreambuf_iterator<char>(file), {});

  nv::IRuntime *runtime = nv::createInferRuntime(logger);
  nv::ICudaEngine *engine = runtime->deserializeCudaEngine(data.data(), data.size());
  nv::IExecutionContext *context = engine->createExecutionContext();

  void *buffers[3];
  int planes_idx = engine->getBindingIndex("planes");
  int policy_idx = engine->getBindingIndex("policy");
  int value_idx = engine->getBindingIndex("value");
  std::vector<float> planes(49*5*5, 0.0f);
  std::vector<float> policy(180, 0.0f);
  std::vector<float> value(1, 0.0f);

  cudaSafeCall(cudaMalloc(&buffers[planes_idx], planes.size() * sizeof(float)));
  cudaSafeCall(cudaMalloc(&buffers[policy_idx], policy.size() * sizeof(float)));
  cudaSafeCall(cudaMalloc(&buffers[value_idx], value.size() * sizeof(float)));

  cudaSafeCall(cudaMemcpy(buffers[planes_idx], planes.data(), planes.size()*sizeof(float), cudaMemcpyHostToDevice));
  if (context->execute(1, buffers)) {

    cudaSafeCall(cudaMemcpy(policy.data(), buffers[policy_idx], policy.size() * sizeof(float), cudaMemcpyDeviceToHost));
    cudaSafeCall(cudaMemcpy(value.data(), buffers[value_idx], value.size() * sizeof(float), cudaMemcpyDeviceToHost));
    std::cout << "pi: ";
    std::cout.precision(3);
    std::cout << std::fixed;
    for (int i = 0, n = policy.size(); i < n; i++) {
      if (policy[i] > 0.0f) {
        std::cout << i << ":" << policy[i] << " ";
      }
    }
    std::cout << std::endl;
    std::cout << "v:  " << value[0] << std::endl;
  }

  cudaSafeCall(cudaFree(buffers[0]));
  cudaSafeCall(cudaFree(buffers[1]));
  cudaSafeCall(cudaFree(buffers[2]));

  context->destroy();
  engine->destroy();
  runtime->destroy();
  ::google::ShutdownGoogleLogging();

  return 0;
}