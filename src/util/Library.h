#pragma once

#include "platform.h"

#if defined(__LINUX__)
#include <dlfcn.h>
#elif defined(__WIN__)
#include <Windows.h>
#endif
#include <string>

#include "marcos.h"

class Library
{
public:
  Library();
  ~Library();
  bool load(const char* name);
  
  // void* getModule(const char *name);

  template <typename FnPtr>
  NODISCARD FnPtr getModule(const char *name);
  void close();

  std::string lastError() const;
  std::string getDLLName() const { return dll_name_; }
private:
  std::string dll_name_;
#if defined(__LINUX__)
  void* module_{ nullptr };
#elif defined(__WIN__)
  HMODULE module_{ nullptr };
#endif
  bool hasModel_{false};
};
