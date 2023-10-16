#pragma once
#include "TcpConnection.h"
#include "net/Connector.h"
#include "net/EventLoop.h"

class TcpClient
{
public:
	// TcpClient(EventLoop *eventLoop, TcpClientEnv &env);
	TcpClient(EventLoop *eventLoop);
  virtual ~TcpClient();

	virtual bool start(const char *ip, uint16_t port);
	virtual bool stop();

protected:
	virtual TcpConnection::ptr onConnect(sockfd_t fd);
protected:
	// TcpClientEnv &env_;
	EventLoop *event_loop_;

	std::unique_ptr<Connector> connector_;
	TcpConnection::ptr conn_;

	std::atomic_bool running_{false};
	Mutex::type mutex_;
};
