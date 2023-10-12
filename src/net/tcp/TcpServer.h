#pragma once
#include "net/Acceptor.h"
#include "env/ServerEnv.h"
#include "net/tcp/TcpConnection.h"
#include "net/EventLoop.h"

class TcpServer
{
public:
	TcpServer(EventLoop *eventLoop);
	virtual ~TcpServer();

	virtual bool start(const char *ip, uint16_t port, int max_backlog);
	virtual bool stop();

protected:
	virtual TcpConnection::ptr onConnect(sockfd_t fd);
	virtual void addConnection(sockfd_t fd, TcpConnection::ptr conn);
	virtual void removeConnection(sockfd_t fd);

	// TcpServerEnv &env_;
	EventLoop *event_loop_;
	std::unique_ptr<Acceptor> acceptor_;

	std::unordered_map<sockfd_t, TcpConnection::ptr> connections_;

	std::atomic_bool running_;
	Mutex::type mutex_;
};
