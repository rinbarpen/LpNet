#include "Connector.h"

#include "net/tcp/TcpSocket.h"
#include "net/udp/UdpSocket.h"

Connector::Connector(TaskScheduler::ptr taskScheduler) :
  task_scheduler_(taskScheduler), next_retry_interval_(500), retry_times_(5),
  socket_(new TcpSocket())
{
}

Connector::~Connector()
{
  
}

bool Connector::connect(const char* ip, uint16_t port)
{
  if (connecting_ || retry_times_ <= 0) return false;

  sockfd_t server_fd = socket_api::connect(socket_->getSockfd(), ip, port);
  if (server_fd < 0) {
    task_scheduler_->addTriggerEventForce([&]() {
      next_retry_interval_ *= 2;
      if (next_retry_interval_ > 30000) {
        next_retry_interval_ = 30000;
      }
      if (!this->connect(ip, port)) {
        retry_times_--;
      }
    }, next_retry_interval_);
  } else {
    connect_cb_(server_fd);
    next_retry_interval_ = 500;
    retry_times_ = 5;
    connecting_ = true;

    return true;
  }

  return false;
}

void Connector::close()
{
  if (connecting_) {
    if (socket_ && socket_->isValid()) {
      socket_->close();
    }

    if (disconnect_cb_) {
      disconnect_cb_();
    }

    connecting_ = false;
  }
}
