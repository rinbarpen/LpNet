#pragma once
#include "TcpConnection.h"
#include "net/EventLoop.h"

class TcpClient
{
public:
	// TcpClient(EventLoop *event_loop, TcpClientEnv &env);
	TcpClient(EventLoop *event_loop);
  virtual ~TcpClient();

	virtual bool start(const char *ip, uint16_t port);
	virtual bool stop();

protected:
	virtual TcpConnection::ptr onConnect(sockfd_t fd);
protected:
	// TcpClientEnv &env_;
	EventLoop *event_loop_;

	TcpConnection::ptr conn_;

	std::atomic_bool running_;
	Mutex::type mutex_;
};
