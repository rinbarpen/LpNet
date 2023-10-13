#include "Acceptor.h"

Acceptor::Acceptor(TaskScheduler::ptr taskScheduler) :
  task_scheduler_(taskScheduler)
{
  accept_cb_ = [this]() {
    this->onAccept();
  };
}
void Acceptor::listen(const char* ip, uint16_t port, int backlog)
{
  this->listen({ip, port}, backlog);
}
void Acceptor::listen(const NetAddress& addr, int backlog)
{
  LOG_DEBUG() << "Acceptor is listening(" << backlog << ") "
              << "on " << "(" << addr.ip << ", " << addr.port << ")";
  Mutex::lock locker(mutex_);
  if (nullptr == socket_)
    socket_.reset(new TcpSocket());
  else if (socket_->type() == "TcpSocket")
    socket_.reset(new TcpSocket());
  else if (socket_->type() == "UdpSocket")
    socket_.reset(new UdpSocket());
  else
    socket_.reset(new TcpSocket());

  socket_api::setReuseAddr(socket_->getSockfd());
  socket_api::setReusePort(socket_->getSockfd());
  socket_api::setNonBlocking(socket_->getSockfd());

  if (socket_->bind(addr) < 0) {
    throw std::runtime_error("bind");
  }
  channel_.reset(new Channel(socket_->getSockfd()));
  if (socket_->listen(backlog) < 0) {
    throw std::runtime_error("listen");
  }

  channel_->setReadCallback([this]() { this->onAccept(); });
  channel_->enableReading();
  task_scheduler_->updateChannel(channel_);
}

void Acceptor::close()
{
  LOG_DEBUG() << "Acceptor is closed";

  Mutex::lock locker(mutex_);
  if (socket_->isValid()) {
    task_scheduler_->removeChannel(channel_->getSockfd());
    socket_->close();
  }
}

void Acceptor::onAccept()
{
  Mutex::lock locker(mutex_);
  auto &&[target, _] = socket_->accept();
  LOG_DEBUG() << "Acceptor accepts a new client: " << target;
  if (target != kInvalidSocket) {
    if (new_connection_cb_) new_connection_cb_(target);
    else socket_api::close(target);
  }
}
