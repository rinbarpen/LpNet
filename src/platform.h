#pragma once

#if defined(WIN32) || defined(_WIN32)
#ifndef __WIN__
#define __WIN__
#define WIN32_LEAN_AND_MEAN
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#pragma comment(lib, "Ws2_32.lib")

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

#endif
#endif

#include <cstdint>
#include <cstring>