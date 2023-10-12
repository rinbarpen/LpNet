#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <thread>

#include "marcos.h"
#include "Mutex.h"
#include "logger/Logger.h"
#include "util/Clock.h"

// TODO: Fix the bug that the Cycle Task cannot be called
using TimerCallBackFn = std::function<void()>;
using TimerTaskId = uint64_t;
// TODO: Test: cycle
struct TimerTask
{
  uint64_t duration;
  uint64_t expired_time;
  bool cycled{false};
  TimerCallBackFn callback;
  TimerTaskId id;

  TimerTask() = default;
  TimerTask(uint64_t _duration, bool _cycled = false) :
    duration(_duration), cycled(_cycled)
  {
    expired_time = Clock::now<T_high_resolution_clock>() + duration;
    id = next_time_id();
  }
  TimerTask(TimerCallBackFn _callback, uint64_t _duration, bool _cycled = false) :
    callback(_callback),
    duration(_duration), cycled(_cycled)
  {
    expired_time = Clock::now<T_high_resolution_clock>() + duration;
    id = next_time_id();
  }

  void setTimerCallback(TimerCallBackFn fn) { callback = fn; }
  bool isExpired() const
  {
    auto now = Clock::now<T_high_resolution_clock>();
    return now >= expired_time;
  }
  uint64_t getExpireTime() const { return expired_time; }
  void setExpiredTime(const uint64_t duration)
  {
    this->duration = duration;
    expired_time = Clock::now<T_high_resolution_clock>() + duration;
  }

  bool isCycle() const { return cycled; }
  void setCycle(bool _cycled) { cycled = _cycled; }

  void run()
  {
    if (isExpired()) {
      LOG_DEBUG() << "A callback Fn has been called";
      callback();
    }
  }
  inline void operator()() { if(callback) run(); }

  bool operator< (const TimerTask &other) const
  {
    return this->expired_time < other.expired_time;
  }
  bool operator>=(const TimerTask &other) const
  {
    return !(*this < other);
  }
  bool operator> (const TimerTask &other) const
  {
    return expired_time > other.expired_time;
  }
  bool operator<=(const TimerTask &other) const
  {
    return !(*this > other);
  }
  bool operator==(const TimerTask &other) const
  {
    return this->id == other.id;
  }
  bool operator!=(const TimerTask &other) const
  {
    return !(*this == other);
  }

  static inline uint64_t curr_timer_id = 1;
  static uint64_t next_time_id() { return curr_timer_id++; }
};

class TimerTaskBase
{
public:
  TimerTaskBase() = default;
  virtual ~TimerTaskBase() = default;

  virtual TimerTask top() const = 0;

  virtual bool pop() = 0;
  virtual bool pop(TimerTask &task) = 0;

  virtual TimerTaskId add(const TimerTask &task) = 0;
  virtual bool remove(TimerTaskId id) = 0;
  virtual bool modify(TimerTaskId id, uint64_t duration, bool cycled = false) = 0;

  virtual size_t size() const = 0;
  virtual const size_t capacity() const = 0;
};

// TODO: Add the capacity fun
class TimerTaskHeap : public TimerTaskBase
{
protected:
public:
  TimerTaskHeap(size_t capacity) :
    capacity_(capacity)
  {
    data_.resize(capacity_);
  }
  ~TimerTaskHeap() = default;

  NODISCARD TimerTask top() const override;

  bool pop() override;
  bool pop(TimerTask& task) override;

  TimerTaskId add(const TimerTask &task) override;
  bool remove(TimerTaskId id) override;
  bool modify(TimerTaskId id, uint64_t duration, bool cycled = false) override;

  size_t size() const override { return size_; }
  const size_t capacity() const override { return capacity_; }
private:
  void moveUp(size_t idx);
  void moveDown(size_t idx);

  void swap(size_t x, size_t y);

  size_t indexOf(TimerTaskId id) const;
  size_t indexOf(const TimerTask &task) const;
private:
  std::vector<TimerTask> data_;
  const size_t capacity_;
  size_t size_;

  mutable Mutex::type mutex_;
};

// TODO: Add the capacity fun
class TimerTaskMap : public TimerTaskBase
{
public:
  using IdTimestampPair = std::pair<TimerTaskId, uint64_t>;

  explicit TimerTaskMap(size_t capacity) :
    capacity_(capacity)
  {}
  ~TimerTaskMap() = default;

  TimerTask top() const override;

  bool pop() override;
  bool pop(TimerTask &task) override;

  TimerTaskId add(const TimerTask& task) override;
  bool remove(TimerTaskId id) override;
  bool modify(TimerTaskId id, uint64_t duration, bool cycled = false) override;

  size_t size() const override { return data_.size(); }
  const size_t capacity() const override { return capacity_; }
private:
  NODISCARD IdTimestampPair hash(const TimerTask &task) const;

  struct Comparator {
    bool operator()(const IdTimestampPair &a, const IdTimestampPair &b) const {
      return a.first != b.first && a.second <= b.second;
    }
  };

private:
  std::map<IdTimestampPair, TimerTask, Comparator> data_;
  const size_t capacity_;
};

// TODO: Test this worker
class Timer
{
public:
  Timer(size_t capacity, bool heap = false)
  {
    if (heap)
      tasks_ = new TimerTaskHeap(capacity);
    else
      tasks_ = new TimerTaskMap(capacity);
  }
  ~Timer()
  {
    if (running_) 
      stop();

    if (tasks_) delete tasks_;
  }

  NODISCARD TimerTask getTimer();

  TimerTaskId addTimer(const TimerTask &task);
  bool removeTimer(TimerTaskId id);
  bool modifyTimer(TimerTaskId id, uint64_t duration, bool cycled = false);

  void start();
  void stop();

  /*
   * the next timestamp the timer executed
   */
  uint64_t nextTimestamp();

  size_t size() const { return tasks_->size(); }
  const size_t capacity() const { return tasks_->capacity(); }

private:
  void run()
  {
    uint64_t timestamp{};
    TimerTask task;

    LOG_DEBUG() << "Now is processing the timer tasks";
    if (running_) {
      // {
      //   Mutex::ulock locker(mutex_);
      //   cond_.wait_for(locker, [this]() {
      //     return running_ && tasks_->size() > 0;
      //   }, std::chrono::milliseconds(100));
      // }
      {
        Mutex::lock locker(mutex_);
        LOG_DEBUG() << "Now we has " << tasks_->size() << " task(s)";

        timestamp = this->nextTimestamp();
        if (timestamp == Clock::infinity()) {
          LOG_DEBUG() << "No task to do";
          timestamp = 100;
        }
        if (timestamp) {
          LOG_DEBUG() << "sleep " << timestamp << "(ms)";
          Clock::sleep(timestamp);
        }
      }

      task = this->getTimer();
      task();
    }
  }
private:
  // Con tasks_;
  TimerTaskBase* tasks_;

  std::atomic_bool running_{false};
  std::condition_variable cond_;
  std::thread worker_;

  mutable Mutex::type mutex_;
};

