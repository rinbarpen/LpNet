#include "EventLoop.h"
#include "platform.h"

EventLoop::EventLoop(ThreadPool &threadPool) :
  thread_pool_(threadPool)
{
}
bool EventLoop::addTriggerEvent(const TaskSchedulerId &id, TriggerEventCallback fn)
{
  Mutex::lock locker(mutex_);
  return task_schedulers_[id]->addTriggerEvent(fn);
}
TimerTaskId EventLoop::addTimer(const TaskSchedulerId &id, const TimerTask &task)
{
  Mutex::lock locker(mutex_);
  return task_schedulers_[id]->addTimer(task);
}
void EventLoop::removeTimer(const TaskSchedulerId &id, TimerTaskId timerId)
{
  Mutex::lock locker(mutex_);
  return task_schedulers_[id]->removeTimer(timerId);
}
void EventLoop::updateChannel(const TaskSchedulerId &id, Channel::ptr pChannel)
{
  Mutex::lock locker(mutex_);
  task_schedulers_[id]->updateChannel(pChannel);
}
void EventLoop::removeChannel(const TaskSchedulerId &id, sockfd_t sockfd)
{
  Mutex::lock locker(mutex_);
  task_schedulers_[id]->removeChannel(sockfd);
}
void EventLoop::addTaskScheduler(TaskScheduler::ptr pTaskScheduler)
{
  Mutex::lock locker(mutex_);
  task_schedulers_[pTaskScheduler->getId()] = pTaskScheduler;
}
TaskScheduler::ptr EventLoop::getTaskScheduler(const TaskSchedulerId &id)
{
  Mutex::lock locker(mutex_);
  auto it = task_schedulers_.find(id);
  if (it != task_schedulers_.end())
    return it->second;

  return nullptr;
}

bool EventLoop::hasTaskScheduler(const TaskSchedulerId &id) const {
  Mutex::lock locker(mutex_);
  return task_schedulers_.find(id) != task_schedulers_.end();
}

void EventLoop::run()
{
  LOG_DEBUG() << "EventLoop is running";
  for (auto &[id, pTaskScheduler] : task_schedulers_) {
    thread_pool_.submit([=]() {
      pTaskScheduler->start();
    });
  }
  thread_pool_.start();
  thread_pool_.stop();

  while (true) {
    Clock::sleep(1000);
  }
}