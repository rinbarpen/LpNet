#include "BufferReader.h"

int BufferReader::append(const std::string &data)
{
  int write_len = writableBytes();
  if (write_len < MAX_BYTES_PER_READ) {
    char *nbuf = new char[capacity_ * 2];
    ::memmove(nbuf, data_, capacity_);
    capacity_ *= 2;
    delete[] data_;
    data_ = nbuf;
  }

  int len = data.length();
  ::memcpy(data_, data.c_str(), len);
  if (len <= 0) return 0;

  write_pos_ += len;
  return len;
}
int BufferReader::read(size_t n, std::string &data)
{
  int size = readableBytes();
  if (n > 0 && n <= size) {
    data.assign(data_ + read_pos_, n);
    advance(n);

    return n;
  }
  return 0;
}
/*
int BufferReader::read(sockfd_t fd)
{
  int write_len = writableBytes();
  if (write_len < MAX_BYTES_PER_READ) {
    char *nbuf = new char[capacity_ * 2];
    ::memmove(nbuf, data_, capacity_);
    capacity_ *= 2;
    delete[] data_;
    data_ = nbuf;
  }

  int len = ::net::socket_api::recv(fd, data_ + write_pos_, MAX_BYTES_PER_READ);
  if (len <= 0) return 0;
  
  write_pos_ += len;
  return len;
}
*/
int BufferReader::readAll(std::string &data)
{
  int size = readableBytes();
  data.assign(data_ + read_pos_, size);

  reset();

  return size;
}
void BufferReader::advance(size_t n)
{
  if (n > 0) {
    read_pos_ += n;
    if (write_pos_ == read_pos_)
      reset();
  }
}
void BufferReader::advanceTo(const char *target)
{
  if (target - data_ > 0 && target - data_ <= write_pos_) {
    read_pos_ = target - data_;
    if (write_pos_ == read_pos_) 
      reset();
  }
}
void BufferReader::reset()
{
  read_pos_ = 0;
  write_pos_ = 0;
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
