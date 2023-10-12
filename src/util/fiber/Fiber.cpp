#include "Fiber.h"

#include "defs.h"
#include "marcos.h"

Fiber::Fiber(uint32_t fid, FiberScheduler *pFS) :
  fid_(fid), status_(READY),
  buffer_(nullptr), buf_size_(0),
  buf_cap_(0), pFS_(pFS)
{

}
Fiber::~Fiber()
{
  if (buffer_) delete[] buffer_;
}

void Fiber::resume()
{
  switch (status_) {
  case READY:
    {
#if defined(__LINUX__)
    ::swapcontext(&pFS_->main_ctx, &ctx_);
#endif
    status_ = RUNNING;
    break;
    }
  case SUSPEND:
    {
    loadStack();
#if defined(__LINUX__)
    ::swapcontext(&pFS_->main_ctx, &ctx_);
#endif
    status_ = RUNNING;
    break;
    }
  }  // switch
}
void Fiber::yield()
{
  if (status_ == RUNNING) {
    status_ = SUSPEND;
    saveStack();
#if defined(__LINUX__)
    ::swapcontext(&ctx_, &pFS_->main_ctx);
#endif
  }
}
void Fiber::run()
{
  if (fiber_cbfn_)
    fiber_cbfn_();
}
void Fiber::saveStack()
{
  char *p = pFS_->getStackBottom();
  char dummy = 0;

  int used = p - &dummy;
  LY_ASSERT(used <= kFiberBufferSize);
  if (used > buf_cap_) {
    if (buffer_) delete[] buffer_;
    buf_cap_ = used;
    buffer_ = new char[buf_cap_];
  }

  buf_size_ = used;
  ::memcpy(buffer_, &dummy, buf_size_);
}
void Fiber::loadStack()
{
  void* p = pFS_->getStackBottom() - buf_size_;
  ::memcpy(p, buffer_, buf_size_);
}
