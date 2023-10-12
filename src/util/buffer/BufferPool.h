#pragma once
#include "Replacer.h"


template <class K, class V>
class BufferPool
{
public:
  BufferPool(size_t capacity) :
    replacer_(new LRUReplacer<K, V>(capacity))
  {}
  ~BufferPool()
  {
    if (replacer_) delete replacer_;
  }

  bool access(const K &key, V &value)
  {
    return replacer_->access(key, value);
  }
  bool evite(const K &key, V& value)
  {
    return replacer_->evite(key, value);
  }
  bool remove(const K &key)
  {
    return replacer_->remove(key);
  }

private:
  Replacer<K, V> *replacer_;

};
