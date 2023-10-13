#include "TcpClient.h"

TcpClient::TcpClient(EventLoop* eventLoop) :
  event_loop_(eventLoop)
{
	connector_.reset(new Connector(eventLoop->getTaskScheduler("UdpClient")));

	connector_->setConnectCallback([&](sockfd_t fd) {
	  conn_ = this->onConnect(fd);
	});
	connector_->setDisconnectCallback([this]() {
	  
	});
}
TcpClient::~TcpClient()
{
  
}

bool TcpClient::start(const char* ip, uint16_t port)
{
  if (running_) return false;

	LOG_DEBUG() << "TcpClient::start()";
	connector_->connect(ip, port);
	running_ = true;

  return true;
}
bool TcpClient::stop()
{
  if (!running_) return false;

	LOG_DEBUG() << "TcpClient::stop()";
	conn_->disconnect();

	connector_->close();
	running_ = false;

  return true;
}

TcpConnection::ptr TcpClient::onConnect(sockfd_t fd)
{
	return std::make_shared<TcpConnection>(
	event_loop_->getTaskScheduler("TcpClient").get(), fd);
}
