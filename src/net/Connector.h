#pragma once
#include <functional>

#include <net/socket_util.h>
#include <marcos.h>

#include "net/EventLoop.h"

class Connector
{
public:
  using ConnectCallback = std::function<void(sockfd_t)>;
  using DisconnectCallback = std::function<void()>;

  Connector(TaskScheduler::ptr taskScheduler);
  ~Connector();

  bool connect(const char *ip, uint16_t port);
  void close();

  void setConnectCallback(ConnectCallback fn) { connect_cb_  = fn; }
  void setDisconnectCallback(DisconnectCallback fn) { disconnect_cb_ = fn; }

  bool isConnecting() const { return connecting_; }

  NONCOPYABLE(Connector);
private:
  TaskScheduler::ptr task_scheduler_;

  std::unique_ptr<Socket> socket_;
  NetAddress addr_;
  int next_retry_interval_;
  ConnectCallback connect_cb_;
  DisconnectCallback disconnect_cb_;
  int retry_times_;

  bool connecting_{false};
};

