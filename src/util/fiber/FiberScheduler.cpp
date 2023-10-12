#include "FiberScheduler.h"

#include "3rdparty/openssl/proverr.h"

static void fiberEntry(void* pFiber)
{
  Fiber *co = static_cast<Fiber *>(pFiber);
  co->setStatus(Fiber::RUNNING);
  co->run();
  co->setStatus(Fiber::SUSPEND);
}

FiberScheduler::~FiberScheduler()
{
  for (auto &[_, pFiber] : fibers_) {
    if (pFiber) {
      pFiber->yield();
      pFiber.reset();
    }
  }
}

std::shared_ptr<Fiber> FiberScheduler::fiberNew(FiberScheduler *pFS)
{
  if (nullptr == pFS) return nullptr;

  uint32_t fid = nextFid();
  return std::make_shared<Fiber>(fid, pFS);
}

void FiberScheduler::submit(std::shared_ptr<Fiber> pFiber)
{
  if (pFiber == nullptr)
    pFiber = FiberScheduler::fiberNew(this);

  {
    Mutex::lock locker(mutex_);
    fibers_[pFiber->id()] = pFiber;
  }

  ::getcontext(&pFiber->ctx_);
  pFiber->ctx_.uc_link           = &main_ctx_;
  pFiber->ctx_.uc_stack.ss_flags = 0;
  pFiber->ctx_.uc_stack.ss_size  = kFiberBufferSize;
  pFiber->ctx_.uc_stack.ss_sp    = main_stack_;

  ::makecontext(&pFiber->ctx_, (void(*)()) & fiberEntry, 1, static_cast<void *>(pFiber.get()));
}
bool FiberScheduler::remove(uint32_t fid)
{
  Mutex::lock locker(mutex_);
  auto it = fibers_.find(fid);
  if (it != fibers_.end()) {
    fibers_.erase(it);
    return true;
  }
  return false;
}
bool FiberScheduler::move2otherScheduler(uint32_t fid, FiberScheduler *pFS)
{
  if (!pFS || this == pFS) return false;

  Mutex::lock locker(mutex_);
  auto it = fibers_.find(fid);
  if (it != fibers_.end()) {
    pFS->submit(it->second);
    fibers_.erase(it);
    return true;
  }
  return false;
}
void FiberScheduler::resume(uint32_t fid)
{
  Mutex::lock locker(mutex_);
  if (curr_fid_ == fid) return;
  
  auto it = fibers_.find(fid);
  if (it != fibers_.end()) {
    this->yield();
    it->second->resume();
    curr_fid_ = fid;
  }
}

void FiberScheduler::yield()
{
  Mutex::lock locker(mutex_);
  if (curr_fid_ == kInvalidFiberId)
    return;

  fibers_[curr_fid_]->yield();
  curr_fid_ = kInvalidFiberId;
}

std::shared_ptr<Fiber> FiberScheduler::getNextFiber()
{
  Mutex::lock locker(mutex_);
  if (fiber_queue_.empty()) return nullptr;

  auto fid = fiber_queue_.front(); fiber_queue_.pop();
  return fibers_[fid];
}

uint32_t FiberScheduler::currFiberId() const
{
  Mutex::lock locker(mutex_);

  return curr_fid_;
}
