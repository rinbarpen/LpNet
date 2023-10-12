#pragma once

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <stdexcept>
#include <memory>
#include <string>
#include <tuple>

#define LY_AUTHOR  LpoutYoumu
#define LY_VERSION 01

#define LY_ASSERT(x) assert(x)
#define LY_CHECK(x, err_msg) static_assert(x, err_msg)

class UnreachableException : public std::runtime_error {
public:
  UnreachableException(const char *msg) : std::runtime_error(msg) {}
};

#define UNREACHABLE_THROW()                                                    \
  do {                                                                         \
    char __s[100];                                                             \
    ::snprintf(__s, 100, "Unreachable has been called by %s in %s:%d",         \
               __FUNCTION__, __FILE__, __LINE__);                              \
    throw ::UnreachableException(__s);                                         \
  } while (0)

#define UNREACHABLE_NOTHROW()                                                  \
  do {                                                                         \
    ::fprintf(stderr, "Unreachable has been called by %s in %s:%d",            \
              __FUNCTION__, __FILE__, __LINE__);                               \
  } while (0)

#define UNREACHABLE() UNREACHABLE_THROW()

#define NONCOPYABLE(CLS)                                                      \
  CLS(const CLS &) = delete;                                                   \
  CLS &operator=(const CLS &) = delete

#define NONMOVEABLE(CLS)                                                      \
  CLS(CLS &&) = delete;                                                        \
  CLS &operator=(CLS &&) = delete

#define UNUSED      [[maybe_unused]]
#define NODISCARD   [[nodiscard]]

#define SHARED_PTR_USING(CLS, ALIAS)    using ALIAS = std::shared_ptr<CLS>
#define UNIQUE_PTR_USING(CLS, ALIAS)    using ALIAS = std::unique_ptr<CLS>
#define WEAK_PTR_USING(CLS, ALIAS)      using ALIAS = std::weak_ptr<CLS>
#define SHARED_PTR_TYPEDEF(CLS, ALIAS)  typedef std::shared_ptr<CLS> ALIAS
#define UNIQUE_PTR_TYPEDEF(CLS, ALIAS)  typedef std::unique_ptr<CLS> ALIAS
#define WEAK_PTR_TYPEDEF(CLS, ALIAS)    typedef std::weak_ptr<CLS>   ALIAS

#define MAKE_SHARED(CLS)  \
  template <typename... Args>\
  [[nodiscard]] static inline std::shared_ptr<CLS> make_shared(Args&&... args) { \
    return std::make_shared<CLS>(std::forward<Args>(args)...); \
  }

#define SHARED_REG(CLS) \
  SHARED_PTR_USING(CLS, ptr); \
  MAKE_SHARED(CLS)

#define UNWARP(WRAPPER, TYPE, ...) \
  auto TYPE[__VA_ARGS__] = WRAPPER
