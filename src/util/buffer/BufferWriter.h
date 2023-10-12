#pragma once

#include <memory>
#include <queue>


void writeU16Forward(char *p, uint16_t value);
void writeU16Reverse(char *p, uint16_t value);
void writeU24Forward(char *p, uint32_t value);
void writeU24Reverse(char *p, uint32_t value);
void writeU32Forward(char *p, uint32_t value);
void writeU32Reverse(char *p, uint32_t value);

class BufferWriter
{
public:
  explicit BufferWriter(size_t qcap) :
    capacity_(qcap)
  {}

  bool append(std::shared_ptr<char> data, uint32_t len, uint32_t pos = 0);
  bool append(const char* data, uint32_t len, uint32_t pos = 0);
  // int  send(sockfd_t fd, int ms_timeout = 0);

  bool empty() const { return q_.empty(); }
  bool full()  const { return capacity_ == q_.size(); }
  size_t capacity() const { return capacity_; }
private:
  struct Packet
  {
    std::shared_ptr<char> data;
    uint32_t len;
    uint32_t pos;
  };

  std::queue<Packet> q_;
  size_t capacity_;
};
