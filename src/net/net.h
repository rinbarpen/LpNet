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

#elif defined(__WIN__)
#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#endif
