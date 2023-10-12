#pragma once
#include "TaskScheduler.h"
#include "Mutex.h"

class SelectManager : public TaskScheduler
{
public:
  SHARED_REG(SelectManager);

  SelectManager(TaskSchedulerId id);
  ~SelectManager();

  void updateChannel(Channel::ptr pChannel) override;
  void removeChannel(sockfd_t fd) override;

  bool handleEvent(int ms_timeout = 0) override;
private:
  sockfd_t max_fd_;
  fd_set fd_read_backup_;
  fd_set fd_write_backup_;
  fd_set fd_exp_backup_;

  bool is_fd_read_reset_ = false;
  bool is_fd_write_reset_ = false;
  bool is_fd_exp_reset_ = false;

  Mutex::type mutex_;

  std::unordered_map<sockfd_t, Channel::ptr> channels_;
};

