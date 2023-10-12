#include "BufferWriter.h"

#include <cstring>

#include "net/Acceptor.h"

void writeU16Forward(char *p, uint16_t value)
{
  p[0] = value >> 8;
  p[1] = value & 0xFF;
}
void writeU16Reverse(char *p, uint16_t value)
{
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}
void writeU24Forward(char *p, uint32_t value)
{
  p[0] = value >> 16;
  p[1] = value >> 8;
  p[2] = value & 0xFF;
}
void writeU24Reverse(char *p, uint32_t value)
{
  p[2] = value >> 16;
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}
void writeU32Forward(char *p, uint32_t value)
{
  p[0] = value >> 24;
  p[1] = value >> 16;
  p[2] = value >> 8;
  p[3] = value & 0xFF;
}
void writeU32Reverse(char *p, uint32_t value)
{
  p[3] = value >> 24;
  p[2] = value >> 16;
  p[1] = value >> 8;
  p[0] = value & 0xFF;
}

bool BufferWriter::append(std::shared_ptr<char> data, uint32_t len, uint32_t pos)
{
  if (len <= pos) return false;
  if (full()) return false;
  Packet pkt = {
    data, len, pos
  };
  q_.emplace(pkt);
  return true;
}
bool BufferWriter::append(const char *data, uint32_t len, uint32_t pos)
{
  if (len <= pos) return false;
  if (full()) return false;

  Packet pkt;
  pkt.data.reset(new char[len + 512], std::default_delete<char[]>());
  ::memcpy(pkt.data.get(), data, len);
  pkt.len = len;
  pkt.pos = pos;
  q_.push(pkt);
  return true;
}
/*
int BufferWriter::send(sockfd_t fd, int ms_timeout)
{
  if (ms_timeout > 0)
    ::net::socket_api::setBlocking(fd);

  int ret = 0;
  while (!q_.empty()) {
    Packet& p = q_.front();
    ret = ::send(fd, p.data.get() + p.pos, p.len - p.pos, 0);
    if (ret > 0) {
      p.pos += ret;
      if (p.pos == p.len) q_.pop();
    }
    else {
#if defined(__WIN__)      
      printf("Get Error: %s\n", WSAGetLastError());
#elif defined(__LINUX__)
#endif
    }
  }

  if (ms_timeout > 0)
    ::net::socket_api::setNonBlocking(fd);
}
*/