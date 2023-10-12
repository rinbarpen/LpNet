#include "Arena.h"

#include <cassert>
#include <cstdint>

namespace util
{
Arena::Arena() :
  alloc_ptr_(nullptr), alloc_bytes_remaining_(0),
  memory_usage_(0)
{}

Arena::~Arena()
{
  for (auto block : blocks_) {
    delete[] block;
  }
}

char* Arena::allocateFallBack(size_t nbytes)
{
  if (nbytes > kBlockSize / 4) {
    char *res = allocateNewBlock(nbytes);
    return res;
  }

  alloc_ptr_ = allocateNewBlock(kBlockSize);
  alloc_bytes_remaining_ = kBlockSize;

  char *res = alloc_ptr_;
  alloc_ptr_ += nbytes;
  alloc_bytes_remaining_ -= nbytes;

  return res;
}
char* Arena::allocateNewBlock(size_t block_bytes)
{
  char *res = new char[block_bytes];
  blocks_.push_back(res);
  memory_usage_.fetch_add(block_bytes + sizeof(char*), 
              std::memory_order_relaxed);
  return res;
}


char *Arena::allocate(size_t nbytes)
{
  char *res;
  if (nbytes <= alloc_bytes_remaining_) {
    res = alloc_ptr_;
    alloc_ptr_ += nbytes;
    alloc_bytes_remaining_ -= nbytes;
  }
  else {
    res = allocateFallBack(nbytes);
  }

  return (res);
}

char* Arena::allocateAligned(size_t nbytes)
{
  constexpr int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  static_assert((align & (align - 1)) == 0, 
      "Pointer size should be a power of 2");

  size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
  size_t slop = (current_mod == 0 ? 0 : align - current_mod);
  size_t needed = nbytes + slop;
  char *res;
  if (needed <= alloc_bytes_remaining_) {
    res = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {
    res = allocateFallBack(nbytes);
  }

  assert((reinterpret_cast<uintptr_t>(res) & (align - 1)) == 0);
  return (res);
}

}  // namespace util