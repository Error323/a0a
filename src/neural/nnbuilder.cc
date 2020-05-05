#include <NvInfer.h>
#include <NvInferRuntime.h>
#include <glog/logging.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

constexpr int kFilters = 64;
constexpr int kBlocks = 6;
constexpr int kBatchSize = 1;

namespace nv = nvinfer1;

class WeightList {
 public:
  using weights_t = std::pair<nv::Weights, nv::Weights>;

  WeightList(const std::string& filename) {
    std::ifstream file(filename.c_str());
    std::string line, item;
    float v;
    while (std::getline(file, line)) {
      if (line.find("batch_normalization") != std::string::npos) {
        // bias, mean, stddev
        static const std::vector<std::string> type{"bias", "mean", "sigma"};
        for (int i = 0; i < 3; i++) {
          std::getline(file, line);
          std::stringstream ss(line);
          std::vector<float> row;
          while (std::getline(ss, item, ' ')) {
            v = std::atof(item.c_str());
            row.emplace_back(v);
          }
          VLOG(1) << "BatchNorm " << type[i] << " " << row.size();
          weights_.emplace_back(row);
        }
      } else if (line.find("conv") != std::string::npos) {
        // conv
        std::getline(file, line);
        std::stringstream ss(line);
        std::vector<float> row;
        while (std::getline(ss, item, ' ')) {
          v = std::atof(item.c_str());
          row.emplace_back(v);
        }
        VLOG(1) << "Conv " << row.size();
        weights_.emplace_back(row);
      } else if (line.find("dense") != std::string::npos) {
        // dense, bias
        static const std::vector<std::string> type{"dense", "bias"};
        for (int i = 0; i < 2; i++) {
          std::getline(file, line);
          std::stringstream ss(line);
          std::vector<float> row;
          while (std::getline(ss, item, ' ')) {
            v = std::atof(item.c_str());
            row.emplace_back(v);
          }
          VLOG(1) << "Dense " << type[i] << " " << row.size();
          weights_.emplace_back(row);
        }
      } else {
        LOG(FATAL) << "Invalid weightline " << line;
      }
    }
  }

  weights_t PopConv(nv::DimsNCHW dims) {
    // Extract the convolution kernels and fuse the batchnorm weights
    auto& weights_t = Pop();  // weights
    auto& bias_t = Pop();     // bias
    auto& mean_t = Pop();     // mean
    auto& sigma_t = Pop();    // standard deviation

    // Default tensorflow tf.keras.layers.BatchNormalization epsilon
    constexpr float epsilon = 1e-3f;

    // Compute reciprocal of std-dev from the variances (so that it can be just
    // multiplied).
    for (auto&& w : sigma_t) {
      w = 1.0f / std::sqrt(w + epsilon);
    }

    // 1. Transpose the weights into cudnn format
    //    Tensorflow: [filter_height, filter_width, in_channels, out_channels],
    //    HWIO cudnn: [output, input, filter_height, filter_width], OIHW
    //    tf.transpose(weights, [3, 2, 0, 1])
    //
    // 2. Fuse the batchnorm layer into the convolution weights and bias
    //    see: https://tkv.io/posts/fusing-batchnorm-and-conv.
    std::vector<float> tmp(weights_t);
    nv::Weights weights{nv::DataType::kFLOAT, weights_t.data(),
                        int64_t(weights_t.size())};
    nv::Weights bias{nv::DataType::kFLOAT, bias_t.data(),
                     int64_t(bias_t.size())};
    // clang-format off
    for (int o = 0, O = dims.n(); o < O; o++) {
      for (int i = 0, I = dims.c(); i < I; i++) {
        for (int h = 0, H = dims.h(); h < H; h++) {
          for (int w = 0, W = dims.w(); w < W; w++) {
            weights_t[o*I*H*W + i*H*W + h*W + w] =
                tmp[h*W*I*O + w*I*O + i*O + o] * sigma_t[o];
          }
        }
      }

      bias_t[o] -= mean_t[o] * sigma_t[o];
    }
    // clang-format on
    VLOG(1) << "Popped Conv " << index_ << " " << weights_t.size() << ", "
            << bias_t.size();
    return std::make_pair(weights, bias);
  }

  weights_t PopDense() {
    auto& weights_t = Pop();  // weights
    auto& bias_t = Pop();     // bias

    VLOG(1) << "Popped Dense " << index_ << " " << weights_t.size() << ", "
            << bias_t.size();

    nv::Weights weights{nv::DataType::kFLOAT, weights_t.data(),
                        int64_t(weights_t.size())};
    nv::Weights bias{nv::DataType::kFLOAT, bias_t.data(),
                     int64_t(bias_t.size())};

    return std::make_pair(weights, bias);
  }

 private:
  int index_{0};
  std::vector<std::vector<float>> weights_;
  std::vector<float>& Pop() { return weights_[index_++]; }
};

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

nv::ILayer* Conv(nv::INetworkDefinition* net, WeightList& wl, nv::ILayer* x,
                 int filters, nv::Dims kernel_size) {
  nv::Weights weights, bias;
  std::tie(weights, bias) =
      wl.PopConv(nv::DimsNCHW{filters, x->getOutput(0)->getDimensions().d[1],
                              kernel_size.d[0], kernel_size.d[1]});
  nv::IConvolutionLayer* conv = net->addConvolutionNd(
      *x->getOutput(0), filters, kernel_size, weights, bias);
  conv->setPaddingMode(nv::PaddingMode::kSAME_UPPER);
  x = net->addActivation(*conv->getOutput(0), nv::ActivationType::kRELU);
  return x;
}

nv::ILayer* ResBlock(nv::INetworkDefinition* net, WeightList& wl,
                     nv::ILayer* input, int filters, nv::Dims kernel_size) {
  nv::ILayer* x = Conv(net, wl, input, filters, kernel_size);
  nv::Weights weights, bias;
  std::tie(weights, bias) =
      wl.PopConv(nv::DimsNCHW{filters, x->getOutput(0)->getDimensions().d[1],
                              kernel_size.d[0], kernel_size.d[1]});
  nv::IConvolutionLayer* conv = net->addConvolutionNd(
      *x->getOutput(0), filters, kernel_size, weights, bias);
  conv->setPaddingMode(nv::PaddingMode::kSAME_UPPER);
  x = net->addElementWise(*conv->getOutput(0), *input->getOutput(0),
                          nv::ElementWiseOperation::kSUM);
  x = net->addActivation(*x->getOutput(0), nv::ActivationType::kRELU);
  return x;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    LOG(FATAL) << "no weights file given";
  }

  FLAGS_alsologtostderr = 1;
  FLAGS_v = 2;
  FLAGS_colorlogtostderr = 1;
  ::google::InitGoogleLogging(argv[0]);
  ::google::InstallFailureSignalHandler();

  static Logger logger;
  WeightList wl(argv[1]);

  nv::IBuilder* builder = nv::createInferBuilder(logger);
  nv::INetworkDefinition* net = builder->createNetworkV2(0u);
  nv::IBuilderConfig* config = builder->createBuilderConfig();
  nv::Weights weights, bias;

  // input definition
  nv::ITensor* input = net->addInput("planes", nv::DataType::kFLOAT,
                                     nv::DimsNCHW{kBatchSize, 49, 5, 5});

  // initial convolution
  std::tie(weights, bias) = wl.PopConv(nv::DimsNCHW{64, 49, 3, 3});
  nv::IConvolutionLayer* conv =
      net->addConvolutionNd(*input, kFilters, nv::DimsHW{3, 3}, weights, bias);
  conv->setPaddingMode(nv::PaddingMode::kSAME_UPPER);
  nv::ILayer* x =
      net->addActivation(*conv->getOutput(0), nv::ActivationType::kRELU);

  // residual blocks
  for (int i = 0; i < kBlocks; i++) {
    x = ResBlock(net, wl, x, kFilters, nv::DimsHW{3, 3});
  }

  // FIXME(Folkert): for some reason tensorflow outputs this layer out of order
  // but this is the first value head v
  nv::ILayer* v = Conv(net, wl, x, 32, nv::DimsHW{1, 1});

  // policy head
  nv::ILayer* pi = Conv(net, wl, x, 180, nv::DimsHW{1, 1});
  pi = net->addPoolingNd(*pi->getOutput(0), nv::PoolingType::kAVERAGE,
                         nv::DimsHW{5, 5});
  pi->getOutput(0)->setName("policy");
  net->markOutput(*pi->getOutput(0));

  // value head
  std::tie(weights, bias) = wl.PopDense();
  v = net->addFullyConnected(*v->getOutput(0), 64, weights, bias);
  v = net->addActivation(*v->getOutput(0), nv::ActivationType::kRELU);

  std::tie(weights, bias) = wl.PopDense();
  v = net->addFullyConnected(*v->getOutput(0), 1, weights, bias);
  v = net->addActivation(*v->getOutput(0), nv::ActivationType::kTANH);
  v->getOutput(0)->setName("value");
  net->markOutput(*v->getOutput(0));

  builder->setMaxBatchSize(kBatchSize);
  config->setMaxWorkspaceSize(1024 * 1024 * 1024);

  nv::ICudaEngine* engine = builder->buildEngineWithConfig(*net, *config);

  nv::IHostMemory* buffer = engine->serialize();

  std::ofstream file("network.trt.bin", std::ios::binary);
  file.write(reinterpret_cast<const char*>(buffer->data()), buffer->size());
  file.close();

  buffer->destroy();
  engine->destroy();
  config->destroy();
  net->destroy();
  builder->destroy();
  ::google::ShutdownGoogleLogging();

  return 0;
}
