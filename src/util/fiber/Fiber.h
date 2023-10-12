#pragma once

#include <ucontext.h>

#include <functional>
#include <string>

#include "FiberScheduler.h"


class FiberScheduler;
class Fiber
{
public:
  using FiberCallbackFn = std::function<void()>;

  enum Status
  {
    READY,
    SUSPEND,
    RUNNING,
    DONE,
  };

  Fiber(uint32_t fid, FiberScheduler* pFS);
  ~Fiber();

  void resume();
  void yield();
  void run();

  void setFiberCallbackFn(FiberCallbackFn f) { fiber_cbfn_ = f; }

  uint32_t id() const { return fid_; }
  void setStatus(Status status) { status_ = status; }
  Status getStatus() const { return status_; }
  
private:
  void saveStack();
  void loadStack();
public:
  ucontext_t ctx_;
  char *buffer_;
protected:
  int buf_size_;
  int buf_cap_;
private:
  uint32_t fid_;
  Status status_;
  FiberCallbackFn fiber_cbfn_;

  FiberScheduler* pFS_;
};
