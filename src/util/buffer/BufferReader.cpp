#include "BufferReader.h"

int BufferReader::append(const char *data, size_t len)
{
  int write_len = buffer_.writableBytes();
  if (write_len < Buffer::kMaxBytesPerRead) {
    buffer_.reset(buffer_.capacity() * 2);
  }

  buffer_.write(data, len);
  
  return len;
}
int BufferReader::read(size_t n, std::string &data)
{
  int bytesRead = buffer_.readableBytes();
  if (n > 0 && n <= bytesRead) {
    buffer_.read(data.data(), n);

    return n;
  }

  return 0;
}

int BufferReader::read(sockfd_t fd)
{
  int write_len = writableBytes();
  // ��������
  if (write_len < Buffer::kMaxBytesPerRead) {
    buffer_.reset(buffer_.capacity() * 2);
  }

  int len = buffer_.write(fd, Buffer::kMaxBytesPerRead);
  if (len <= 0) return 0;

  return len;
}

int BufferReader::readAll(std::string &data)
{
  int size = buffer_.readableBytes();
  buffer_.read(data.data(), size);

  return size;
}
void BufferReader::advance(size_t n)
{
  buffer_.read(nullptr, n);
}
void BufferReader::advanceTo(const char *target)
{
  buffer_.readTo(target);
}
void BufferReader::reset()
{
  buffer_.clear();
}

uint16_t readU16Forward(const char *p)
{
  uint16_t res = 0;

  res |= p[0] << 8;
  res |= p[1];
  return res;
}
uint16_t readU16Reverse(const char *p)
{
  uint16_t res = 0;

  res |= p[1] << 8;
  res |= p[0];
  return res;
}
uint32_t readU24Forward(const char *p)
{
  uint32_t res = 0;

  res |= p[0] << 16;
  res |= p[1] << 8;
  res |= p[2];
  return res;
}
uint32_t readU24Reverse(const char *p)
{
  uint32_t res = 0;

  res |= p[2] << 16;
  res |= p[1] << 8;
  res |= p[0];
  return res;
}
uint32_t readU32Forward(const char *p)
{
  uint32_t res = 0;

  res |= p[0] << 24;
  res |= p[1] << 16;
  res |= p[2] << 8;
  res |= p[3];
  return res;
}
uint32_t readU32Reverse(const char *p)
{
  uint32_t res = 0;

  res |= p[3] << 24;
  res |= p[2] << 16;
  res |= p[1] << 8;
  res |= p[0];
  return res;
}
