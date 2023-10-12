#pragma once

#include "marcos.h"
#include "Mutex.h"
#include "net/net.h"
#include "net/TaskScheduler.h"
#include "util/ThreadPool.h"
// #include "util/fiber/FiberScheduler.h"


class EventLoop
{
public:
  SHARED_REG(EventLoop);
  using TriggerEventCallback = std::function<void()>;
  using TaskSchedulerMap = std::unordered_map<TaskSchedulerId, TaskScheduler::ptr>;

  EventLoop(ThreadPool &threadPool);

  // 都不会检查目标TaskScheduler是否存在
  bool addTriggerEvent(const TaskSchedulerId &id, TriggerEventCallback fn);
  TimerTaskId addTimer(const TaskSchedulerId &id, const TimerTask &task);
  void removeTimer(const TaskSchedulerId &id, TimerTaskId timerId);
  void updateChannel(const TaskSchedulerId &id, Channel::ptr pChannel);
  void removeChannel(const TaskSchedulerId &id, sockfd_t sockfd);

  void addTaskScheduler(TaskScheduler::ptr pTaskScheduler);
  TaskScheduler::ptr getTaskScheduler(const TaskSchedulerId &id);

  bool hasTaskScheduler(const TaskSchedulerId &id) const;

  void run();

  NONCOPYABLE(EventLoop);
private:
  mutable Mutex::type mutex_;

  TaskSchedulerMap task_schedulers_;

  // FiberScheduler::ptr pFiberScheduler_;

  ThreadPool &thread_pool_;
};

