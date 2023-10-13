#pragma once
#include "marcos.h"
#include "net/Channel.h"
#include "net/TaskScheduler.h"
#include "net/Socket.h"
#include "net/tcp/TcpSocket.h"
#include "net/udp/UdpSocket.h"

class Acceptor
{
public:
  SHARED_REG(Acceptor);
  using AcceptCallbackFn = std::function<void()>;
  using NewConnectionCallback = std::function<void(sockfd_t)>;

  Acceptor(TaskScheduler::ptr taskScheduler);
  ~Acceptor() = default;

  void listen(const char *ip, uint16_t port, int backlog);
  void listen(const NetAddress &addr, int backlog);
  void close();

  void setNewConnectionCallback(const NewConnectionCallback &fn) { new_connection_cb_ = fn; }
  // bool isIpv6() const { return socket_->isIpv6(); }

  std::string getIp() const { return socket_->getIp(); }
  uint16_t getPort() const { return socket_->getPort(); }
private:
  void onAccept();

private:
  TaskScheduler::ptr task_scheduler_;
  std::unique_ptr<Socket> socket_;
  AcceptCallbackFn accept_cb_;
  NewConnectionCallback new_connection_cb_;
  Channel::ptr channel_;

  Mutex::type mutex_;
};

