#pragma once
#include <mutex>
#include <shared_mutex>

class Mutex
{
public:
  using type = std::mutex;
  using lock = std::lock_guard<type>;
  using ulock = std::unique_lock<type>;

};

class RWMutex
{
public:
  using type = std::shared_mutex;
  using rlock = std::shared_lock<type>;
  using wlock = std::unique_lock<type>;

};

// TODO: Complete me!
class Sem
{
public:

private:
  int cnt_{0};
};

using sem_t = Sem;