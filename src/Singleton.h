#pragma once

template <class T>
class Singleton
{
public:
  virtual ~Singleton() = default;
  static T* instance() 
  {
    if (!pValue_)
      pValue_ = new T();
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
  static T *pValue_;
};

template <class T>
T *Singleton<T>::pValue_ = nullptr;
