#pragma once

#include <functional>
#include <unordered_map>
#include <queue>
#include <memory>

#include "defs.h"
#include "Fiber.h"
#include "marcos.h"
#include "Mutex.h"

class Fiber;
class FiberScheduler
{
protected:
public:
  using ptr        = std::shared_ptr<FiberScheduler>;
  using CallbackFn = std::function<void()>;
  using FiberMap   = std::unordered_map<uint32_t, std::shared_ptr<Fiber>>;
  using FiberQueue = std::queue<uint32_t>;

  explicit FiberScheduler(std::string schedulerName) :
    name_(schedulerName)
  {}
  ~FiberScheduler();

  void run() { if(cbfn_) cbfn_(); }
  void setCallbackFn(CallbackFn fn) { cbfn_ = fn; }

  static std::shared_ptr<Fiber> fiberNew(FiberScheduler* pFS);

  void submit(std::shared_ptr<Fiber> pFiber);
  bool remove(uint32_t fid);

  bool move2otherScheduler(uint32_t fid, FiberScheduler *pFS);

  void resume(uint32_t fid);
  void yield();

  // robin-round
  std::shared_ptr<Fiber> getNextFiber();
  uint32_t currFiberId() const;

  const std::string& name() const { return name_; }
  char *getStackBottom() { return main_stack_ + kFiberBufferSize; }

  ucontext_t main_ctx_;
  char main_stack_[kFiberBufferSize];

  NONCOPYABLE(FiberScheduler);
private:
  static uint32_t nextFid() { return next_fid_++; }
  static inline uint32_t next_fid_ = 0;

private:
  uint32_t curr_fid_{ kInvalidFiberId };

  FiberMap fibers_;

  FiberQueue fiber_queue_;

  CallbackFn cbfn_;
  std::string name_;

  mutable Mutex::type mutex_;
};

static void fiberEntry(void *pFiber);
