#pragma once
#include <list>
#include <mutex>
#include <vector>
#include <atomic>

#include "marcos.h"

namespace util
{
// TODO: Implement part
// Implement incr Arena
// for these fixed Containers
class Arena
{
protected:
  static constexpr int kBlockSize = 4096;
public:
  Arena();
	~Arena();

  char *allocate(size_t nbytes);
  char *allocateAligned(size_t nbytes);

  size_t memoryUsage() const { return memory_usage_.load(std::memory_order_relaxed); }

  // void free(char *p, size_t nbytes);

  NONCOPYABLE(Arena);
private:
  char *allocateFallBack(size_t nbytes);
  char *allocateNewBlock(size_t block_bytes);
private:
  char *alloc_ptr_{nullptr};
  size_t alloc_bytes_remaining_{0};

  std::vector<char*> blocks_;
  // std::vector<char*> free_blocks_;

  std::atomic<size_t> memory_usage_{0};
};

static Arena arena;

}  // namespace util
