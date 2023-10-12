#include "TcpServer.h"

TcpServer::TcpServer(EventLoop *eventLoop):
  event_loop_(eventLoop)
{
	acceptor_ = std::make_unique<Acceptor>(eventLoop->getTaskScheduler("TcpServer"));
	acceptor_->setNewConnectionCallback([this](sockfd_t fd) {
		TcpConnection::ptr conn = this->onConnect(fd);
	  if (conn) {
		  this->addConnection(fd, conn);
		  conn->setDisconnectCallback([this](TcpConnection::ptr conn) {
			  auto scheduler = conn->getTaskScheduler();
		    sockfd_t sockfd = conn->getSockfd();
				scheduler->addTriggerEventForce([this, sockfd] {
				  this->removeConnection(sockfd);
				}, 100);  // TODO: Replace with env.removeConnectionMsTimeout
		  });
	  }
	});
}

TcpServer::~TcpServer()
{
	stop();
}

bool TcpServer::start(const char *ip, uint16_t port, int max_backlog)
{
	if (running_) return false;

	LOG_DEBUG() << "TcpServer::start()";
	acceptor_->listen(ip, port, max_backlog);
	running_ = true;
	return true;
}

bool TcpServer::stop()
{
	if (!running_) return false;

	LOG_DEBUG() << "TcpServer::stop()";
  {
		Mutex::lock locker(mutex_);
		for (auto &[fd, conn] : connections_) {
			conn->disconnect();
		}
	}

	acceptor_->close();
	running_ = false;

	for(;;) {
		if (connections_.empty()) break;
		Clock::sleep(10);
	}
  return true;
}

TcpConnection::ptr TcpServer::onConnect(sockfd_t fd)
{
	LOG_DEBUG() << fd << " connects";
	return std::make_shared<TcpConnection>(event_loop_->getTaskScheduler("TcpServer").get(), fd);
}

void TcpServer::addConnection(sockfd_t fd, TcpConnection::ptr conn)
{
	Mutex::lock locker(mutex_);
	connections_.emplace(fd, conn);
}

void TcpServer::removeConnection(sockfd_t fd)
{
	Mutex::lock locker(mutex_);
	connections_.erase(fd);
}
