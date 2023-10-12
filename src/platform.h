#pragma once

#if defined(WIN32) || defined(_WIN32)
#ifndef __WIN__
#define __WIN__
#define WIN32_LEAN_AND_MEAN
#define FD_SETSIZE      1024
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
typedef SOCKET sockfd_t;

#pragma comment(lib, "ws2_32.lib")

#define SHUT_RD 0
#define SHUT_WR 1 
#define SHUT_RDWR 2
#endif

#elif defined(__linux) || defined(__linux__)
#ifndef __LINUX__
#define __LINUX__

#include <sys/types.h>         
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <netinet/ether.h>   
#include <netinet/ip.h>  
#include <netpacket/packet.h>   
#include <arpa/inet.h>
#include <net/ethernet.h>   
#include <net/route.h>  
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef int sockfd_t;
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)

#endif
#endif

#include <cstdint>
#include <cstring>