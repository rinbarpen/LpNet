#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <type_traits>
#include <algorithm>
#include <cstring>
#include <cstdarg>

//#include <ranges>
//#include <span>

#include "platform.h"

namespace string_api
{
//using std::string;

// 1k tags 11ms-16ms
static std::vector<std::string> split(const std::string& s, const std::string& delim) 
{
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = 0;

  while ((end = s.find(delim, start)) != std::string::npos) {
    result.emplace_back(s.substr(start, end - start));
    start = end + delim.length();
  }

  result.emplace_back(s.substr(start));

  return result;
}
static bool start_with(const std::string& s, const std::string& matchStr) {
  size_t len = matchStr.length();
  if (len > s.length()) return false;

  for (size_t i = 0; i < len; ++i) {
    if (s[i] != matchStr[i]) return false;
  }
  return true;
}
static bool end_with(const std::string& s, const std::string& matchStr) {
  size_t len = matchStr.length();
  size_t slen = s.length();
  if (len > slen) return false;

  for (size_t i = 0; i < len; ++i) {
    if (s[slen - 1 - i] != matchStr[i]) return false;
  }
  return true;
}

template<typename... Args>
static std::string append(Args&&... args) {
  std::stringstream ss;
  ((ss << args), ...);
  return ss.str();
}
static size_t find_case(const std::string &s, const std::string& target, size_t start = 0) 
{
  size_t slen = s.length(), tlen = target.length();
  size_t i = 0, j = 0;
  while (i < slen && j < tlen) {
    if (std::tolower(s[i]) == std::tolower(target[j])) {
      ++i; ++j;
    }
    else {
      j = 0;
    }
  }

  if (j >= tlen) return i - tlen;
  return std::string::npos;
}
static std::string& trim_left(std::string &s)
{
  if (s.empty()) return s;

  size_t i = s.find_first_not_of(' ');
  if (i == std::string::npos) {
    s = "";
    return s;
  }
  s = std::move(s.substr(i));
  return s;
}
static std::string& trim_right(std::string &s)
{
  if (s.empty()) return s;

  size_t i = s.find_last_not_of(' ');
  if (i == std::string::npos) {
    s = "";
    return s;
  }
  s = std::move(s.substr(0, i + 1));
  return s;
}
static std::string& trim(std::string &s)
{
  if (s.empty()) return s;

  size_t l = s.find_first_not_of(' ');
  if (l == std::string::npos) {
    s = "";
    return s;
  }
  size_t r = s.find_last_not_of(' ');

  if (l <= r) {
    s = std::move(s.substr(l, r - l + 1));
  }
  return s;
}
static std::string& tolower(std::string &s)
{
#if __cplusplus >= 202002L
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
#else
  for (auto& c : s) {
    c = std::tolower(c);
  }
#endif
  return s;
}
static std::string& toupper(std::string &s)
{
#if __cplusplus >= 202002L
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
#else
  for (auto &c : s) {
    c = std::toupper(c);
  }
#endif
  return s;
}

static std::unordered_map<std::string, std::string>
parse_url_params(const std::string &url)
{
  std::unordered_map<std::string, std::string> ret;

  auto &&paramString = url.substr(url.find_first_of('?') + 1);
  auto &&params = string_api::split(paramString, "&");
  for (const auto &param : params) {
    size_t split_index = param.find_first_of('=');
    ret.emplace(param.substr(0, split_index),
                param.substr(split_index + 1));
  }

  return ret;
}

}  // namespace string_api
