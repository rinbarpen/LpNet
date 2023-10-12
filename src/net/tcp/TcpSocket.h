#pragma once
#include "net/Socket.h"

class TcpSocket : public Socket
{
public:
  TcpSocket() :
    Socket(NetProtocol{NetProtocol::TCP})
  {}

  int setNoDelay(int on = 1)
  {
    int ret = -1;
#ifdef TCP_NODELAY
    ret = setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
    return ret;
  }

  std::string type() const override { return "TcpSocket"; }

  NONCOPYABLE(TcpSocket);
};

