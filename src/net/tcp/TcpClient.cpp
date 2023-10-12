#include "TcpClient.h"

TcpConnection::ptr TcpClient::onConnect(sockfd_t fd)
{
	return std::make_shared<TcpConnection>(
	event_loop_->getTaskScheduler("TcpClient").get(), fd);
}
