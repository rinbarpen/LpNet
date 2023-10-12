#include "SharedString.h"

#include <cstring>

SharedString::SharedString()
{
  
}

SharedString::SharedString(size_t capacity)
{
  capacity_ = capacity;
  data_ = std::shared_ptr<char>{ new char[capacity_], std::default_delete<char[]>() };
}

SharedString::~SharedString()
{
}

void SharedString::reset()
{
  data_.reset();
  size_ = 0;
  capacity_ = 0;
}

void SharedString::append(const char* data, size_t len)
{
  if (capacity_ - size_ < len) {
    capacity_ = size_ + len;
    std::shared_ptr<char> new_data{new char[capacity_], std::default_delete<char[]>()};
    memcpy(new_data.get(), data_.get(), size_);
    memcpy(new_data.get() + size_, data, len);
    size_ += len;
    data_ = new_data;
  } else {
    memcpy(data_.get() + size_, data, len);
    size_ += len;
  }
}
