#pragma once
#include "net/Socket.h"

class UdpSocket : public Socket
{
public:
  UdpSocket() :
    Socket(NetProtocol{ NetProtocol::UDP })
  {}

  std::string type() const override { return "UdpSocket"; }

  NONCOPYABLE(UdpSocket);
};

