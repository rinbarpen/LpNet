#pragma once

#include <queue>
#include "marcos.h"
#include "Mutex.h"

template <typename T>
class SafeQueue
{
public:
  SafeQueue(size_t capacity) :
    capacity_(capacity)
  {}
  ~SafeQueue() = default;

  bool push(T&& x)
  {
    Mutex::lock locker(mutex_);
    if (fullInternal()) return false;

    q_.push(x);
    return true;
  }

  bool pop(T& x)
  {
    Mutex::lock locker(mutex_);
    if (emptyInternal())
      return false;

    x = q_.front(); q_.pop();
    return true;
  }
  bool pop()
  {
    Mutex::lock locker(mutex_);
    if (emptyInternal())
      return false;

    q_.pop();
    return true;
  }


  size_t capacity() const { return capacity_; }
  size_t size() const { Mutex::lock locker(mutex_); return sizeInternal(); }
  bool empty()  const { Mutex::lock locker(mutex_); return emptyInternal(); }
  bool full()   const { Mutex::lock locker(mutex_); return fullInternal(); }

  NONCOPYABLE(SafeQueue);
private:
  size_t sizeInternal() const { return q_.size(); }
  bool emptyInternal()  const { return q_.empty(); }
  bool fullInternal()   const { return q_.size() == capacity_; }

private:
  const size_t capacity_;
  std::queue<T> q_;

  mutable Mutex::type mutex_;
};

