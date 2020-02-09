#pragma once

#include <switch.h>

#include <atomic>
#include <functional>

#include "logger.hpp"

template <typename>
class LoopThread;

template <typename A>
void loop_thread(void *arg) {
  auto loop_thread = reinterpret_cast<LoopThread<A> *>(arg);
  if (loop_thread->_parent_loop) {
    while (loop_thread->_active) {
      loop_thread->_function(loop_thread->_arg);
    }
  } else {
    loop_thread->_function(loop_thread->_arg);
  }
}

template <typename A>
class LoopThread {
 private:
  using Executor = std::function<void(A)>;

  A _arg;
  Executor _function;
  Thread _loop_thread{};
  std::atomic_bool _active;
  bool _parent_loop;

  template <typename>
  friend void loop_thread(void *arg);

 public:
  LoopThread(A arg, Executor function, size_t stack_size,
             bool parent_loop = true, int prio = 0x2C, int cpuid = -2)
      : _arg(arg),
        _function(std::move(function)),
        _active(false),
        _parent_loop(parent_loop) {
    Result rc = threadCreate(&_loop_thread, loop_thread<A>, this, nullptr,
                             stack_size, prio, cpuid);
    if (R_FAILED(rc)) {
      Logger::write("threadCreate() returned 0x%x\n", rc);
    }
  }

  ~LoopThread() {
    stop();
    Result rc = threadClose(&_loop_thread);
    if (R_FAILED(rc)) {
      Logger::write("threadClose() returned 0x%x\n", rc);
    }
  }

  void start() {
    if (!_active.exchange(true)) {
      Result rc = threadStart(&_loop_thread);
      if (R_FAILED(rc)) {
        _active = false;
        Logger::write("threadStart() returned 0x%x\n", rc);
      }
    }
  }

  void stop(const std::function<void()> &stopper = []() {}) {
    if (_active.exchange(false)) {
      stopper();
      Result rc = threadWaitForExit(&_loop_thread);
      Logger::write("threadWaitForExit() returned 0x%x\n", rc);
    }
  }

  [[nodiscard]] inline bool isActive() const { return _active; }
};
