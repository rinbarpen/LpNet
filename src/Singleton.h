#pragma once

template <class T>
class Singleton
{
public:
  virtual ~Singleton() = default;
  static T* instance() 
  {
    if (!pValue_)
      std::call_once(once_, [&]() {
        pValue_ = new T();
      });
    return pValue_;
  }
  
private:
  Singleton() = default;
  
  struct Deletor {
    ~Deletor()
    {
      if (pValue_)
        delete pValue_;
    }
  };

  Deletor deletor_;
  static inline std::once_flag once_ = {};
  static inline T *pValue_ = nullptr;
};

// template <class T>
// T *Singleton<T>::pValue_ = nullptr;
