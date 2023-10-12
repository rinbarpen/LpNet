#pragma once

#include <chrono>

template <class ClockType = std::chrono::steady_clock>
class Timestamp {
public:

  void reset()
  {
    begin_tp_ = ClockType::now();
  }
  int64_t elapsed()
  {
    auto end_tp = ClockType::now();
    int64_t elapse = std::chrono::duration_cast<std::chrono::milliseconds>(end_tp - begin_tp_).count();
    begin_tp_ = end_tp;
    return elapse;
  }

  int64_t now()
  {
    return ClockType::now().time_since_epoch().count();
  }
private:
  std::chrono::time_point<ClockType> begin_tp_;
};