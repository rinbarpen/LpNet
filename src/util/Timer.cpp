#include "Timer.h"

#include "logger/Logger.h"

TimerTask TimerTaskHeap::top() const
{
  Mutex::lock locker(mutex_);
  if (data_.empty())
    return {};
  return data_[0];
}
bool TimerTaskHeap::pop()
{
  if (size_ == 0) return false;

  remove(data_[0].id);
  return true;
}
bool TimerTaskHeap::pop(TimerTask& task)
{
  if (size_ == 0) return false;

  task = data_[0];
  remove(data_[0].id);
  if (task.isCycle()) {
    task.setExpiredTime(task.duration);
    add(task);
  }
  return true;
}

TimerTaskId TimerTaskHeap::add(const TimerTask& task)
{
  // if (capacity_ <= size_) return;

  data_[size_++] = task;
  moveUp(size_ - 1);

  return task.id;
}
bool TimerTaskHeap::remove(TimerTaskId id)
{
  auto idx = indexOf(id);
  if (idx == -1) return false;

  size_t pa = idx;
  size_t child = idx * 2 + 1;
  while (child < size_) {
    if (child + 1 < size_ && data_[child] > data_[child + 1]) {
      child++;
    }
    this->swap(child, pa);
    pa = child;
    child = pa * 2 + 1;
  }

  size_--;
  return true;
}
bool TimerTaskHeap::modify(TimerTaskId id, uint64_t duration, bool cycled)
{
  size_t idx = indexOf(id);
  if (idx >= size_) return false;

  data_[idx].setExpiredTime(duration);
  data_[idx].setCycle(cycled);

  if (idx == 0) {
    moveDown(idx);
    return true;  
  }

  size_t pa = (idx - 1) / 2;
  if (data_[pa] < data_[idx]) {
    moveDown(idx);
  } else {
    moveUp(idx);
  }
  return true;

  //
  // TimerTask task = data_[idx];
  // task.setExpiredTime(duration);
  // task.setCycle(cycled);
  // remove(id);
  // add(task);
  //
  // return true;
}

void TimerTaskHeap::moveUp(size_t idx)
{
  if (idx == 0) return;

  size_t cur = idx;
  size_t pa = (cur - 1) / 2;
  while (cur > 0 && data_[cur] < data_[pa]) {
    this->swap(cur, pa);

    cur = pa;
    pa = (cur - 1) / 2;
  }
}
void TimerTaskHeap::moveDown(size_t idx)
{
  size_t cur = idx;
  size_t child = cur * 2 + 1;
  while (child < size_) {
    if (child + 1 < size_ && data_[child] > data_[child + 1]) {
      child++;
    }

    if (data_[child] < data_[cur]) {
      this->swap(child, cur);
    } else {
      break;
    }
  }
}
void TimerTaskHeap::swap(size_t x, size_t y)
{
  TimerTask tmp = data_[x];
  data_[x] = data_[y];
  data_[y] = tmp;
}

size_t TimerTaskHeap::indexOf(TimerTaskId id) const
{
  size_t pa = 0;

  while (pa < size_) {
    if (data_[pa].id == id) {
      return pa;
    }
    pa++;
  }

  return -1;
}
size_t TimerTaskHeap::indexOf(const TimerTask& task) const
{
  size_t cur = 0;

  while (cur < size_) {
    if (data_[cur] == task) {
      return cur;
    } else if (data_[cur] > task) {
      break;
    }
    cur = cur * 2 + 1;
  }

  return -1;
}

// TimerTaskMap
// private part
NODISCARD TimerTaskMap::IdTimestampPair TimerTaskMap::hash(const TimerTask &task) const
{
  return {task.id, task.expired_time};
}
// public part
TimerTaskId TimerTaskMap::add(const TimerTask &task)
{
  auto key = hash(task);
  data_[key] = task;
  return key.first;
}
bool TimerTaskMap::remove(TimerTaskId id)
{
  auto it = std::find_if(data_.begin(), data_.end(), 
    [&](const auto &d) {
    return d.first.first == id;
  });
  if (it != data_.end()) {
    data_.erase(it);
    return true;
  }

  return false;
}
bool TimerTaskMap::modify(TimerTaskId id, uint64_t duration, bool cycled)
{
  auto it = std::find_if(data_.begin(), data_.end(),
                         [&](const auto &d) {
    return d.first.first == id;
  });
  if (it != data_.end()) {
    TimerTask task = it->second;
    task.setExpiredTime(duration);
    task.setCycle(cycled);
    data_.erase(it);
    data_.emplace(hash(task), std::move(task));
    return true;
  }

  return false;
}

TimerTask TimerTaskMap::top() const
{
  if (data_.empty()) return {};

  auto it = data_.begin();
  return it->second;
}

bool TimerTaskMap::pop()
{
  if (data_.empty()) return false;

  // cond_.wait(locker, [&]{ return !data_.empty(); })

  data_.erase(data_.begin());
  return true;
}
bool TimerTaskMap::pop(TimerTask& task)
{
  if (data_.empty()) return false;

  auto it = data_.begin();
  task = it->second;
  data_.erase(it);

  if (task.isCycle()) {
    task.setExpiredTime(task.duration);
    data_[hash(task)] = task;
  }
  
  return true;
}

// Timer
TimerTask Timer::getTimer()
{
  TimerTask task;

  {
    Mutex::lock locker(mutex_);
    tasks_->pop(task);
  }
  
  return task;
}

TimerTaskId Timer::addTimer(const TimerTask& task)
{
  Mutex::lock locker(mutex_);
  bool inserted = tasks_->add(task);

  return inserted;
}

bool Timer::removeTimer(TimerTaskId id)
{
  Mutex::lock locker(mutex_);
  return tasks_->remove(id);
}

bool Timer::modifyTimer(TimerTaskId id, uint64_t duration, bool cycled)
{
  Mutex::lock locker(mutex_);

  return tasks_->modify(id, duration, cycled);
}

void Timer::start()
{
  if (running_) return;

  LOG_DEBUG() << "";
  worker_ = std::thread(&Timer::run, this);

  running_ = true;
}

void Timer::stop()
{
  if (!running_) return;

  LOG_DEBUG() << "";
  if (worker_.joinable())
    worker_.join();

  running_ = false;
}

uint64_t Timer::nextTimestamp()
{
  auto task = tasks_->top();
  uint64_t expiredTime = task.getExpireTime();
  if (expiredTime == 0)
    return Clock::infinity();

  auto now = Clock::now<T_high_resolution_clock>();
  return expiredTime >= now ? expiredTime - now : 0;
}
