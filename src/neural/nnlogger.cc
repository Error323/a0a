#include "nnlogger.h"

void Logger::log(Severity severity, const char* msg) {
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
