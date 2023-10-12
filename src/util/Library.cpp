#include "Library.h"

Library::Library() :
  hasModel_(false)
{
}
Library::~Library()
{
  if (hasModel_) close();
}
bool Library::load(const char *name)
{
  dll_name_ = name;
#if defined(__LINUX__)
  module_ = ::dlopen(dll_name_.c_str(), RTLD_LAZY);
  if (module_) hasModel_ = true;
#elif defined(__WIN__)
  module_ = ::LoadLibrary((LPCWSTR)dll_name_.c_str());
  if (module_) hasModel_ = true;
#endif
  
  return hasModel_;
}
template<typename FnPtr>
FnPtr Library::getModule(const char *name)
{
  FnPtr handle;
#if defined(__LINUX__)
  handle = ::dlopen(name, RTLD_LAZY);
#elif defined(__WIN__)
  handle = ::GetProcAddress(module_, name);
#endif
  return handle;
}
template<>
void* Library::getModule(const char *name)
{
  void *handle;
#if defined(__LINUX__)
  handle = ::dlopen(name, RTLD_LAZY);
#elif defined(__WIN__)
  handle = ::GetProcAddress(module_, name);
#endif
  return handle;
}

void Library::close()
{
#if defined(__LINUX__)
  ::dlclose(pModule_);
#elif defined(__WIN__)
  ::FreeLibrary(module_);
#endif
  hasModel_ = false;
}
std::string Library::lastError() const
{
#if defined(__LINUX__)
  return ::dlerror();
#elif defined(__WIN__)
  return "";
#endif
  return "";
}
