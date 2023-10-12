#include "TcpConnection.h"
TcpConnection::TcpConnection(TaskScheduler* pTaskScheduler, sockfd_t fd) :
  task_scheduler_(pTaskScheduler),
  read_buffer_(new Buffer(kMaxBufferSize)),
  write_buffer_(new Buffer(kMaxBufferSize)),
  channel_(new Channel(fd))
{
  channel_->setReadCallback([this]() { this->onRead(); });
  channel_->setWriteCallback([this]() { this->onWrite(); });
  channel_->setCloseCallback([this]() { this->onClose(); });
  channel_->setErrorCallback([this](){ this->onError(); });

  socket_api::setNonBlocking(fd);
  socket_api::setSendBufSize(fd, 100 * 1024);
  socket_api::setKeepAlive(fd);

  channel_->enableReading();
  task_scheduler_->updateChannel(channel_);
}

TcpConnection::~TcpConnection()
{
  this->close();
}

void TcpConnection::send(const char* data, size_t len)
{
  if (running_) {
    {
      Mutex::lock locker(mutex_);
      write_buffer_->write(data, len);
    }

    this->onWrite();
  }
}

void TcpConnection::disconnect()
{
  auto conn = shared_from_this();
  Mutex::lock locker(mutex_);
  task_scheduler_->addTriggerEventForce([conn]() {
    conn->close();
  }, 0);

}

void TcpConnection::onRead()
{
  Mutex::lock locker(mutex_);
  if (!running_) return;

  int ret = read_buffer_->read(channel_->getSockfd());
  if (ret <= 0) {
    this->close();
    return;
  }

  if (read_cb_) {
    bool ret = read_cb_(shared_from_this(), *read_buffer_);
    if (false == ret) {
      this->close();
      return;
    }
  }
}
void TcpConnection::onWrite()
{
  Mutex::lock locker(mutex_);
  if (!running_) return;

  int ret = 0;
  
  ret = write_buffer_->write(channel_->getSockfd(), 100);
  if (ret < 0) {
    this->close();
    return;
  }

  if (write_buffer_->empty()) {
    if (channel_->isWriting()) {
      channel_->disableWriting();
      // task_scheduler_->updateChannel(channel_);
    }
  }
  else if (!channel_->isWriting()) {
    channel_->enableWriting();
    // task_scheduler_->UpdateChannel(channel_);
  }
}
void TcpConnection::onClose()
{
  Mutex::lock locker(mutex_);
  this->close();
}
void TcpConnection::onError()
{
  Mutex::lock locker(mutex_);
  this->close();
}
void TcpConnection::onDisconnect()
{
  Mutex::lock locker(mutex_);
  this->close();
}

void TcpConnection::close()
{
  if (running_) {
    running_ = false;
    task_scheduler_->removeChannel(channel_->getSockfd());

    if (close_cb_) {
      close_cb_(shared_from_this());
    }

    if (disconnect_cb_) {
      disconnect_cb_(shared_from_this());
    }
  }
}
