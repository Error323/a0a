#pragma once

#include <NvInferRuntimeCommon.h>
#include <glog/logging.h>

namespace nv = nvinfer1;

class Logger : public nv::ILogger {
 public:
  void log(Severity severity, const char* msg) override;
};