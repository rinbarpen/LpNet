#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "marcos.h"
#include "Mutex.h"
#include "SafeQueue.h"

class ThreadPool
{
public:
  struct TaskWrapper
  {
    using CallbackFn = std::function<void()>;
    CallbackFn f_;

    TaskWrapper() {}

    template <typename F, typename... Args>
    TaskWrapper(F &&f, Args&&... args)
    {
      f_ = [&]() {
        std::forward<F>(f)(std::forward<Args>(args)...);
      };
    }

    // TaskWrapper(const TaskWrapper &) = default;
    // TaskWrapper(TaskWrapper &&) = default;
    // TaskWrapper &operator=(const TaskWrapper&) = default;
    // TaskWrapper &operator=(TaskWrapper &&) = default;

    void operator()() const
    {
      if (f_) f_();
    }
  };
  
public:
  ThreadPool(size_t capacity, size_t queue_capacity = -1) :
    q_(queue_capacity), capacity_(capacity)
  {
    if (const size_t max_capacity = std::thread::hardware_concurrency();
        capacity_ > max_capacity) {
      capacity_ = max_capacity;
    }
    threads_.resize(capacity_);
    for (auto &th : threads_) {
      th = std::thread(&ThreadPool::work, this);
    }

    running_ = true;
  }
  ~ThreadPool()
  {
    this->stop();
  }

  template <typename F, typename... Args>
  void submit(F&& f, Args&&... args)
  {
    q_.push(TaskWrapper(std::forward<F>(f), std::forward<Args>(args)...));
    cond_.notify_one();
  }
  
  void start()
  {
    if (running_) return;

    LOG_DEBUG() << "";
    for (auto &th : threads_) {
      th = std::thread(&ThreadPool::work, this);
    }

    running_ = true;
  }
  void stop()
  {
    if (!running_) return;

    LOG_DEBUG() << "";
    cond_.notify_all();
    for (auto &th : threads_) {
      if (th.joinable())
        th.join();
    }
    running_ = false;
  }

  size_t capacity() const { return capacity_; }

  NONCOPYABLE(ThreadPool);
private:
  void work()
  {
    while (running_) {
      LOG_DEBUG() << "Now there are " << q_.size() << " task(s)";
      {
        Mutex::ulock locker(mutex_);
        cond_.wait(locker, [this]() {
          return !q_.empty() && running_;
        });
      }
      LOG_DEBUG() << "This thread wanna get a task";
      TaskWrapper task;
      if (q_.pop(task))
        task();
    }
  }

private:
  std::atomic_bool running_{false};
  std::vector<std::thread> threads_;

  SafeQueue<TaskWrapper> q_;
  size_t capacity_;
  mutable Mutex::type mutex_;  
  std::condition_variable cond_;
};

