#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <exception>

namespace utils {
using namespace std::literals::chrono_literals;

template <class T>
class SafeQueue {
 public:
  SafeQueue(void) : q_(), m_(), c_() {}

  ~SafeQueue(void) { Stop(); }

  void Stop() {
    std::lock_guard<std::mutex> lock(m_);
    stop_ = true;
  }

  void Enqueue(T t) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(t);
    c_.notify_one();
  }

  T Dequeue(void) {
    std::unique_lock<std::mutex> lock(m_);
    c_.wait(lock, [&]() { return !q_.empty() || stop_; });

    if (stop_) {
      throw std::runtime_error("Queue stopped");
    }

    T val = q_.front();
    q_.pop();
    return val;
  }

 private:
  std::queue<T> q_;
  std::mutex m_;
  std::condition_variable c_;
  bool stop_{false};
};

}  // namespace utils
