#include "TaskScheduler.h"

#include "defs.h"
#include "platform.h"
#if defined(__LINUX__)
#include <signal.h>
#endif

TaskScheduler::TaskScheduler(TaskSchedulerId id) :
  id_(id),
	wakeup_pipe_(new Pipe()),
	trigger_events_(new RingBuffer<TriggerEvent>(kMaxTriggetEvents)),
	timers_(-1, false)
{
	if (wakeup_pipe_->create()) {
		wakeup_channel_.reset(new Channel(wakeup_pipe_->readFd()));
		wakeup_channel_->enableReading();
		wakeup_channel_->setReadCallback([this]() { this->wake(); });
	}
}

void TaskScheduler::start()
{
	if (running_) return;

	LOG_DEBUG() << "TaskScheduler is starting...";
  running_ = true;

	while (running_) {
		this->timers_.start();
	  this->handleTriggerEvent();
		uint64_t timeout = timers_.nextTimestamp();
		LOG_DEBUG() << "timeout(ms): " << timeout;
	  this->handleEvent(static_cast<int>(timeout));
		this->timers_.stop();
	}
}

void TaskScheduler::stop()
{
	if (!running_) return;

	running_ = false;
	char event = kTriggetEvent;
	wakeup_pipe_->write(&event, 1);
}

TimerTaskId TaskScheduler::addTimer(TimerTask task)
{
	Mutex::lock locker(mutex_);
	auto id = timers_.addTimer(task);
	return id;
}

void TaskScheduler::removeTimer(TimerTaskId timerId)
{
	Mutex::lock locker(mutex_);
	timers_.removeTimer(timerId);
}

bool TaskScheduler::addTriggerEvent(TriggerEvent callback)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(callback);
		char event = TaskScheduler::kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	return false;
}

bool TaskScheduler::addTriggerEventForce(TriggerEvent cb, int ms_timeout)
{
	if (trigger_events_->size() < kMaxTriggetEvents) {
		Mutex::lock locker(mutex_);
		trigger_events_->push(cb);
		char event = kTriggetEvent;
		wakeup_pipe_->write(&event, 1);
		return true;
	}

	TimerTaskId id = this->addTimer(TimerTask([cb] {
		cb();
	}, ms_timeout, false));

	return id != kInvalidTimerId;
}
