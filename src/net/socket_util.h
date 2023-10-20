#pragma once
#include "NetAddress.h"
#include "net/net.h"
#include "util/logger/Logger.h"

struct NetDomain
{
  enum Type
  {
    NONE = 0,
    IPV4 = AF_INET,
    IPV6 = AF_INET6,
  } type;

  bool isIpv4() const { return type == IPV4; }
  bool isIpv6() const { return type == IPV6; }

  static NetDomain toDomain(std::string name)
  {
    if (name == "IPV4") return {IPV4};
    else if (name == "IPV6") return {IPV6};
    return {NONE};
  }
  static std::string toString(NetDomain domain)
  {
    switch (domain.type) {
    case IPV4:
      return "IPV4";
    case IPV6:
      return "IPV6";
    }

    return "NONE";
  }
};
struct NetProtocol
{
  enum Type
  {
    NONE = 0,
    UDP = IPPROTO_UDP,
    TCP = IPPROTO_TCP,
  } type;

  bool isUdp() const { return type == UDP; }
  bool isTcp() const { return type == TCP; }

  static NetProtocol toProcotol(std::string name)
  {
    if (name == "UDP") return {UDP};
    else if (name == "TCP") return {TCP};
    return {NONE};
  }
  static std::string toString(NetProtocol protocol)
  {
    switch (protocol.type) {
    case UDP:
      return "UDP";
    case TCP:
      return "TCP";
    }

    return "NONE";
  }
};

static constexpr sockfd_t kInvalidSocket = -1;
namespace socket_api
{

static sockfd_t socket(int domain, int type, int protocol)
{
  return ::socket(domain, type, protocol);
}
static int bind(sockfd_t fd, const char *ip, uint16_t port)
{
  int r;

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
  // addr.sin_addr.s_addr = inet_addr(ip);
  // r = addr.sin_addr.s_addr;
  r = ::inet_pton(AF_INET, ip, &addr.sin_addr);
  if (r < 0) {
    LOG_ERROR() << "error ip: " << ip;
    return r;
  }

  r = ::bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  if (r == SOCKET_ERROR) {
    LOG_ERROR() << "error bind: (" << inet_ntoa(addr.sin_addr) << " " << ntohs(addr.sin_port) << ")";
    return r;
  }

  {
    LOG_INFO() << "bind: (" << inet_ntoa(addr.sin_addr) << " " << ntohs(addr.sin_port) << ")";
  }
  
  return r;
}
static int listen(sockfd_t fd, int backlog)
{
  int r;
  r = ::listen(fd, backlog);
  if (r < 0) {
    LOG_ERROR() << "error listen: " << fd << " " << backlog;
    return r;
  }
  return r;
}
static sockfd_t socket_bind_listen(int domain, int type, int protocol, 
                              const char *ip, uint16_t port, 
                              int backlog)
{
  sockfd_t fd = socket(domain, type, protocol);
  int r{};
  r = bind(fd, ip, port);
  if (r < 0)
    return kInvalidSocket;
  r = listen(fd, backlog);
  if (r < 0)
    return kInvalidSocket;

  return fd;
}

static sockfd_t accept(sockfd_t fd, char *ip, uint16_t *port)
{
  struct sockaddr_storage addr;
  socklen_t len;
  sockfd_t clientfd = ::accept(fd, (struct sockaddr*) &addr, &len);
  if (clientfd < 0) return clientfd;

  struct sockaddr_in *addr4 = reinterpret_cast<struct sockaddr_in *>(&addr);
  *port = ::ntohs(addr4->sin_port);
  if (::inet_ntop(AF_INET, &addr4->sin_addr, ip, sizeof(ip)) == nullptr) {
    LOG_ERROR() << "error ip: " << ip;
    return kInvalidSocket;
  }
  return clientfd;
}
static int connect(sockfd_t fd, const char *ip, uint16_t port)
{
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = ::htons(port);
  if (::inet_pton(AF_INET, ip, &addr.sin_addr) < 0) {
    LOG_ERROR() << "error ip: " << ip;
    return -1;
  }

  return ::connect(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
}

static int send(sockfd_t fd, const char *buf, uint32_t len, int flags = 0)
{
  return ::send(fd, buf, len, flags);
}
static int recv(sockfd_t fd, char *buf, uint32_t len, int flags = 0)
{
  return ::recv(fd, buf, len, flags);
}
static int send2(sockfd_t fd, const char *buf, uint32_t len, int flags = 0)
{
  int size = ::send(fd, buf, len, flags);
  if (size < 0) {
    return ::send(fd, buf, len, flags);
  }
  return size;
}
static int recv2(sockfd_t fd, char *buf, uint32_t len, int flags = 0)
{
  int size = ::recv(fd, buf, len, flags);
  if (size < 0) {
    return ::recv(fd, buf, len, flags);
  }
  return size;
}

static int close(sockfd_t fd)
{
  LOG_DEBUG() << "close " << fd;
#if defined(__LINUX__)
  return ::close(fd);
#elif defined(__WIN__)
  return ::closesocket(fd);
#endif
}

static int sendto(sockfd_t fd, const char *buf, uint32_t len, int flags, const char *ip, uint16_t port)
{
  int r;
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  r = ::inet_pton(AF_INET, ip, &addr.sin_addr);
  if (r < 0) {
    LOG_ERROR() << "error ip: " << ip;
    return r;
  }

  r = ::sendto(fd, buf, len, flags, (struct sockaddr*)&addr, INET_ADDRSTRLEN);
  return r;
}

static NetAddress getSocketAddr(sockfd_t fd)
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (0 != getsockname(fd, (struct sockaddr *)&addr, &len)) {
    return {};
  }

  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  return { ip, ::ntohs(addr.sin_port) };
}

static NetAddress getPeerAddr(sockfd_t fd)
{
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  if (0 != getpeername(fd, (struct sockaddr*)&addr, &len)) {
    return {};
  }

  char ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
  return { ip, ::ntohs(addr.sin_port) };
}

static void setKeepAlive(sockfd_t fd, int on = 1)
{
  ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)(&on), sizeof(on));
}
static void setReuseAddr(sockfd_t fd, int on = 1)
{
  ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)(&on), sizeof(on));
}
static void setReusePort(sockfd_t fd, int on = 1)
{
#ifdef SO_REUSEPORT
  setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char *)&on, sizeof(on));
#endif	
}
static void setNonBlocking(sockfd_t fd)
{
#if defined(__LINUX__)
  int flags = ::fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  ::fcntl(fd, F_SETFL, flags);
#elif defined(__WIN__)
  u_long on = 1;
  ::ioctlsocket(fd, FIONBIO, &on);
#endif
}

static void setBlocking(sockfd_t fd, int write_ms_timeout = 100)
{
#if defined(__LINUX__)
  int flags = ::fcntl(fd, F_GETFL);
  flags &= ~O_NONBLOCK;
  ::fcntl(fd, F_SETFL, flags);
#elif defined(__WIN__)
  u_long on = 0;
  ::ioctlsocket(fd, FIONBIO, &on);
#endif
  if (write_ms_timeout > 0)
  {
#ifdef SO_SNDTIMEO
#if defined(__LINUX__)
    struct timeval tv = { write_ms_timeout / 1000, (write_ms_timeout % 1000) * 1000 };
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
#elif defined(__WIN__)
    unsigned long ms = (unsigned long)write_ms_timeout;
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ms, sizeof(unsigned long));
#endif		
#endif
  }
}

static void setSendBufSize(sockfd_t fd, int size)
{
  ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size));
}
static void setRecvBufSize(sockfd_t fd, int size)
{
  ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size));
}

static sockfd_t socket_tcp()
{
  sockfd_t fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // setKeepAlive(fd);
  // setReuseAddr(fd);
  // setReusePort(fd);
  // setNonBlocking(fd);
  LOG_DEBUG() << "create a new tcp socket: " << fd;
  return fd;
}
static sockfd_t socket_udp()
{
  LOG_DEBUG() << "";
  sockfd_t fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  // setReuseAddr(fd);
  // setReusePort(fd);
  return fd;
}
}