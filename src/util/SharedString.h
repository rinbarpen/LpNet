#pragma once
#include <memory>

class SharedString final
{
public:
  SharedString();
  SharedString(size_t capacity);
  ~SharedString();

  void reset();

  void append(const char *data, size_t len);
private:
  std::shared_ptr<char> data_;
  size_t size_{0};
  size_t capacity_{0};
};
