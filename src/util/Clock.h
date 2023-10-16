#pragma once

#include <cstdint>
#include <chrono>
#include <cstring>
#include <thread>

// for event
using T_system_clock = std::chrono::system_clock;
using T_steady_clock = std::chrono::steady_clock;
using T_high_resolution_clock = std::chrono::high_resolution_clock;

class Clock
{
public:
  enum Level
  {
    Second,      // s
    MilliSecond, // ms
    MicroSecond, // 
    NanoSecond,  // ns
  };
  static uint64_t zero() { return 0; }
  static uint64_t infinity() { return ~0; }

  template <class ClockType>
  static uint64_t now(Level level = Level::MilliSecond) {
    auto tp = ClockType::now();
    switch (level)
    {
    case Second:
      return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    case MilliSecond:
      return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    case MicroSecond:
      return std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
    case NanoSecond:
      return std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
    }

    return tp.time_since_epoch().count();
  }

  template <class TimeType = std::chrono::milliseconds>
  static void sleep(uint64_t t)
  {
    std::this_thread::sleep_for(TimeType(t));
  }

  static std::pair<int, std::string> currentDay(const char *fmt = "%Y-%m-%d %H:%M:%S")
  {
    time_t t = T_system_clock::to_time_t(T_system_clock::now());
    struct tm tm = *localtime(&t);

    std::string res;
    std::strftime(res.data(), 20, fmt, &tm);
    return { tm.tm_yday, res };
  }
};
