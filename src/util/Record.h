#pragma once
#include <chrono>
#include <iostream>

#include "Singleton.h"

class Record
{
public:
  using clock_type = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<clock_type>;

  void start() { start_tp_ = clock_type::now(); }
  void end() { end_tp_  = clock_type::now(); }
  int64_t duration() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
      end_tp_ - start_tp_)
      .count();
  }
private:
  time_point start_tp_, end_tp_;
};

using SingleRecord = Singleton<Record>;

#define RECORD_ON() \
  SingleRecord::instance()->start()
#define RECORD_OFF() \
  SingleRecord::instance()->end()
#define RECORD_SHOW() \
  SingleRecord::instance()->duration()

#define RECORD(body) \
  do { \
    RECORD_ON(); \
      body \
    RECORD_OFF(); \
    std::cout << "THIS RECORD COST " << RECORD_SHOW() << "(ms)\n"; \
  } while(0)
