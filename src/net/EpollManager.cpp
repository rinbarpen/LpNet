#include "EpollManager.h"

EpollManager::EpollManager(TaskSchedulerId id):
  TaskScheduler(id)
{
#if defined(__LINUX__)
  epfd_ = ::epoll_create1(0);
#endif
}

EpollManager::~EpollManager()
{
#if defined(__LINUX__)
  if (epfd_ > 0) ::close(epfd_);
#endif
}

int EpollManager::control(sockfd_t sockfd, int op, int events)
{
#if defined(__LINUX__)
  int err;
  struct epoll_event event;
  ::memset(&event, 0, sizeof(event));

  {
    Mutex::lock locker(mutex_);
    event.events = events;
    err = ::epoll_ctl(epfd_, op, sockfd, &event);
    if (err < 0) {
      LOG_ERROR_A("EpollManager") << "epoll_ctl error in EpollManager::control";
      return err;
    }

    if (op == EventCTL::DEL)
      this->removeChannel(sockfd);
    else
      channels_[sockfd]->setEvents(events);
  }

  return err;
#else
  return -1;
#endif
}
void EpollManager::updateChannel(Channel::ptr pChannel)
{
  Mutex::lock locker(mutex_);
  channels_[pChannel->getSockfd()] = pChannel;
}
void EpollManager::removeChannel(sockfd_t sockfd)
{
  Mutex::lock locker(mutex_);
  if (auto it = channels_.find(sockfd);
      it != channels_.end()) {
    channels_.erase(it);
  }
}
bool EpollManager::handleEvent(int ms_timeout)
{
#if defined(__LINUX__)
  const int MAX_EVENTS = 100;
  struct epoll_event events[MAX_EVENTS];

  Mutex::lock locker(mutex_);
  int num_events = ::epoll_wait(epfd_, events, MAX_EVENTS, ms_timeout);
  if (num_events < 0) {
    if (errno != EINTR) {
      return false;
    }
  }

  for (int i = 0; i < num_events; ++i) {
    sockfd_t fd = events[i].data.fd;
    channels_[fd]->handleEvent(events[i].events);
  }
  return true;
#else
  return false;
#endif
}