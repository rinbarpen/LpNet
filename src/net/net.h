#pragma once
#include "platform.h"

#if defined(__LINUX__)
#include <sys/signal.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

typedef int sockfd_t;
#define INVALID_SOCKET  ~(0)
#define SOCKET_ERROR    (-1)

#elif defined(__WIN__)
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Winerror.h>

typedef SOCKET sockfd_t;
#define FD_SETSIZE      1024
#define SHUT_RD 0
#define SHUT_WR 1 
#define SHUT_RDWR 2

#endif
