#include "Replacer.h"

template <class K, class V>
bool LRUReplacer<K, V>::access(const K &key, V &value) {
  std::lock_guard<std::mutex> locker(mutex_);
  for (auto it = caches_.begin(); it != caches_.end(); ++it) {
    if (it->first == key) {
      value = it->second;
      caches_.push_front(*it);
      caches_.erase(it);
      return true;
    }
  }

  if (caches_.size() >= this->capacity())
    caches_.pop_back();

  caches_.emplace_front(key, value);
  return false;
}

template <class K, class V>
bool LRUReplacer<K, V>::evite(const K& key, V& value) {
  std::lock_guard<std::mutex> locker(mutex_);

  for (auto it = caches_.begin(); it != caches_.end(); ++it) {
    if (it->first == key) {
      value = it->second;
      caches_.erase(it);
      return true;
    }
  }

  return false;
}

template <class K, class V>
bool LRUReplacer<K, V>::remove(const K& key) {
  std::lock_guard<std::mutex> locker(mutex_);

  for (auto it = caches_.begin(); it != caches_.end(); ++it) {
    if (it->first == key) {
      caches_.erase(it);
      return true;
    }
  }

  return false;
}

template <class K, class V>
size_t LRUReplacer<K, V>::size() const {
  std::lock_guard<std::mutex> locker(mutex_);
  return caches_.size();
}
