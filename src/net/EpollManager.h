#pragma once
#include "net/net.h"
#include "net/Channel.h"
#include "net/TaskScheduler.h"

class EpollManager final : public TaskScheduler
{
public:
  SHARED_REG(EpollManager);
#if defined(__LINUX__)
  enum EventCTL {
    ADD = EPOLL_CTL_ADD,
    MOD = EPOLL_CTL_MOD,
    DEL = EPOLL_CTL_DEL,
  };
  enum EventType {
    READABLE = EPOLLIN,
    WRITABLE = EPOLLOUT,
    URGENT_READ = EPOLLPRI,
    ERR = EPOLLERR,
    HUP = EPOLLHUP,
    ETMODE = EPOLLET /* EPOLLLT */, 
    ONE_SHOT = EPOLLONESHOT,
  };
#else
  enum EventCTL {
    // ADD = EPOLL_CTL_ADD,
    // MOD = EPOLL_CTL_MOD,
    // DEL = EPOLL_CTL_DEL,
  };
  enum EventType {
    // READABLE = EPOLLIN,
    // WRITABLE = EPOLLOUT,
    // URGENT_READ = EPOLLPRI,
    // ERR = EPOLLERR,
    // HUP = EPOLLHUP,
    // ETMODE = EPOLLET /* EPOLLLT */,
    // ONE_SHOT = EPOLLONESHOT,
  };
#endif
  EpollManager(TaskSchedulerId id);
  ~EpollManager();

  int control(sockfd_t sockfd, int op, int events);

  void updateChannel(Channel::ptr pChannel) override;
  void removeChannel(sockfd_t sockfd) override;

  bool handleEvent(int ms_timeout = 0) override;
private:
  int epfd_;

  Mutex::type mutex_;

  std::unordered_map<sockfd_t, Channel::ptr> channels_;
};
