#pragma once

#include <switch.h>

#include <atomic>
#include <functional>

#include "logger.h"

template <typename>
class LoopThread;

template <typename A>
void loop_thread(void *arg) {
  auto loop_thread = reinterpret_cast<LoopThread<A> *>(arg);
  while (loop_thread->_active) {
    loop_thread->_function(loop_thread->_arg);
  }
}

template <typename A>
class LoopThread {
 private:
  using Executor = std::function<void(A)>;

  A _arg;
  Executor _function;
  Thread _loop_thread;
  std::atomic_bool _active;

  template <typename>
  friend void loop_thread(void *arg);

 public:
  LoopThread(A arg, Executor function, size_t stack_size)
      : _arg(arg), _function(std::move(function)), _active(false) {
    Result rc = threadCreate(&_loop_thread, loop_thread<A>, this, nullptr,
                             stack_size, 0x2C, -2);
    if (R_FAILED(rc)) {
      Logger::write("threadCreate() returned 0x%x\n", rc);
    }
  }

  ~LoopThread() {
    stop();
    Result rc = threadClose(&_loop_thread);
    Logger::write("threadClose() returned 0x%x\n", rc);
  }

  inline void start() {
    if (!_active.exchange(true)) {
      Result rc = threadStart(&_loop_thread);
      if (R_FAILED(rc)) {
        _active = false;
        Logger::write("threadStart() returned 0x%x\n", rc);
      }
    }
  }

  inline void stop() {
    if (_active.exchange(false)) {
      Result rc = threadWaitForExit(&_loop_thread);
      Logger::write("threadWaitForExit() returned 0x%x\n", rc);
    }
  }
};
