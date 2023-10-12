#pragma once
#include <atomic>
#include <functional>
#include <memory>

#include "marcos.h"
#include "net/Channel.h"
#include "net/Pipe.h"
#include "util/Timer.h"
#include "util/buffer/RingBuffer.h"

using TaskSchedulerId = std::string;
using TriggerEvent = std::function<void()>;
class TaskScheduler
{
protected:
	static const char kTriggetEvent = 1;
	static const char kTimerEvent = 2;
	static const int  kMaxTriggetEvents = 50000;
public:
  SHARED_REG(TaskScheduler);

	TaskScheduler(TaskSchedulerId id);
	virtual ~TaskScheduler() = default;

	void start();
	void stop();
	TimerTaskId addTimer(TimerTask task);
	void removeTimer(TimerTaskId timerId);
	bool addTriggerEvent(TriggerEvent callback);
	bool addTriggerEventForce(TriggerEvent callback, int ms_timeout);

	TaskSchedulerId getId() const { return id_; }

	virtual void updateChannel(Channel::ptr channel) { }
	virtual void removeChannel(sockfd_t fd) { }
	virtual bool handleEvent(int ms_timeout) { return false; }

protected:
	void wake()
	{
		char event[10] = { 0 };
		while (wakeup_pipe_->read(event, 10) > 0);
	}
	void handleTriggerEvent()
	{
		do {
			TriggerEvent callback;
			if (trigger_events_->pop(callback)) {
				callback();
			}
		} while (!trigger_events_->empty());
	}

	TaskSchedulerId id_;

	std::atomic_bool running_{ false };
	std::unique_ptr<Pipe> wakeup_pipe_;
	std::shared_ptr<Channel> wakeup_channel_;
	std::unique_ptr<RingBuffer<TriggerEvent>> trigger_events_;

	Mutex::type mutex_;
	Timer timers_;
};



