#pragma once

#include <string>
#include <memory>
#include <cstring>

uint16_t readU16Forward(const char *p);
uint16_t readU16Reverse(const char *p);
uint32_t readU24Forward(const char *p);
uint32_t readU24Reverse(const char *p);
uint32_t readU32Forward(const char *p);
uint32_t readU32Reverse(const char *p);

class BufferReader
{
protected:
  static constexpr int MAX_BYTES_PER_READ = 4096;
  //static constexpr int MAX_CAPACITY = 1024 * 1024;  // 1MB
public:
  using ptr = std::shared_ptr<BufferReader>;

  BufferReader(size_t init_size = 4096) :
    capacity_(init_size)
  {
    data_ = new char[capacity_];
  }

  virtual ~BufferReader() 
  {
    if (data_) delete[] data_;
  }

  char *peek() { return data_ + read_pos_; }
  const char *peek() const { return data_ + read_pos_; }

  int append(const std::string& data);
  int read(size_t n, std::string& data);

  // int read(sockfd_t fd);
  int readAll(std::string &data);

  void advance(size_t n);
  void advanceTo(const char *target);
  void reset();

  size_t capacity() const { return capacity_; }
  size_t readableBytes() const { return write_pos_ - read_pos_; }
  size_t writableBytes() const { return capacity_ - write_pos_; }

protected:
  char *data_{nullptr};

  size_t read_pos_{0};
  size_t write_pos_{0};
  size_t capacity_;
};
