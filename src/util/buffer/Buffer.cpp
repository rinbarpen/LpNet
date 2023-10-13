#include "Buffer.h"

#include <cassert>
#include <cstring>

Buffer::Buffer(size_t capacity) :
  capacity_(capacity)
{
  data_ = new char[capacity + 1];
}

Buffer::~Buffer()
{
  delete[] data_;
}

size_t Buffer::read(char *s, size_t n)
{
  // assert(sizeof(s) >= n);
  size_t bytesRead = readableBytes();
  if (bytesRead > n) {
    bytesRead = n;
  }

  if (s) {
    for (size_t i = 0; i < bytesRead; ++i)
      s[i] = data_[indexOf(get_pos_, i)];
  }

  get_pos_ = indexOf(get_pos_, bytesRead);
  return bytesRead;
}

size_t Buffer::write(const char *s, size_t n)
{
  // assert(sizeof(s) <= n);
  size_t bytesWrite = capacity_ - size();
  if (bytesWrite > n) {
    bytesWrite = n;
  }

  if (s) {
    for (size_t i = 0; i < bytesWrite; ++i)
      data_[indexOf(put_pos_, i)] = s[i];
  }

  put_pos_ = indexOf(put_pos_, bytesWrite);
  return bytesWrite;
}

int Buffer::read(sockfd_t fd)
{
  size_t len = writableBytes();
  if (len >= kMaxBytesPerRead) {
    len = kMaxBytesPerRead;
  }

  int bytesWrite = socket_api::recv(fd, data_ + put_pos_, len);
  if (bytesWrite > 0) {
    put_pos_ = indexOf(put_pos_, bytesWrite);
  }

  return bytesWrite;
}

int Buffer::write(sockfd_t fd, size_t size, int ms_timeout)
{
  if (ms_timeout > 0) socket_api::setNonBlocking(fd, false, ms_timeout);
  size_t len = readableBytes();
  if (size < len) {
    len = size;
  }

  int bytesRead = socket_api::send(fd, data_ + get_pos_, len);
  if (bytesRead > 0) {
    get_pos_ = indexOf(get_pos_, bytesRead);
  }

  if (ms_timeout > 0) socket_api::setNonBlocking(fd);
  return bytesRead;
}

void Buffer::reset(const size_t newCapacity)
{
  char *new_data = new char[newCapacity];
  if (nullptr == new_data) {
    throw std::runtime_error("no enough free memory");
  }

  size_t n = size() >= newCapacity ? newCapacity : size();
  for (size_t i = 0; i < n; ++i) {
    new_data[i] = data_[indexOf(get_pos_, i)];
  }

  delete[] data_;
  data_ = new_data;
  get_pos_ = put_pos_ = 0;
  capacity_ = newCapacity;
}

int Buffer::readTo(const char* target)
{
  int len = strlen(target);

  for (size_t i = 0; i < len; ++i) {
    for (size_t j = get_pos_; j != put_pos_; ++j) {
      if (i + j == put_pos_) return -1;

      size_t idx = indexOf(j, i);
      if (data_[idx] != target[i]) {
        break;
      }
    }
  }

  return len;
}

