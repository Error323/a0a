#pragma once

#include <cuda_runtime_api.h>
#include <glog/logging.h>

#define cudaSafeCall(expr) ___cudaSafeCall(expr, __FILE__, __LINE__, __PRETTY_FUNCTION__)

static void ___error(const char *error_string, const char *file, const int line, const char *func) {
    cudaDeviceReset();
    LOG(FATAL) << std::endl
               << "Error: " << error_string << std::endl
               << "  File:  " << file << std::endl
               << "  Line:  " << line << std::endl
               << "  Func:  " << func << std::endl;
}

static inline void ___cudaSafeCall(cudaError_t err, const char *file, const int line,
                                   const char *func = "") {
    if (err != cudaSuccess)
        ___error(cudaGetErrorString(err), file, line, func);
}
