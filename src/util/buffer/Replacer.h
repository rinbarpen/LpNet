#pragma once
#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>

template <class K, class V>
class Replacer
{
public:
  Replacer(size_t capacity) :
    capacity_(capacity)
  {}

  virtual ~Replacer() = default;
  virtual bool access(const K &key, V &value) = 0;
  virtual bool evite(const K &key, V &value) = 0;
  virtual bool remove(const K &key) = 0;

  size_t capacity() const { return capacity_; }

  Replacer(const Replacer&) = delete;
  Replacer& operator=(const Replacer &) = delete;
  Replacer(Replacer &&) = delete;
  Replacer &operator=(Replacer &&) = delete;
protected:
  const size_t capacity_;
};

template <class K, class V>
class LRUReplacer : public Replacer<K, V>
{
public:
  explicit LRUReplacer(size_t capacity) :
    Replacer<K, V>(capacity)
  {}
  ~LRUReplacer() = default;

  bool access(const K &key, V &value) override;
  bool evite(const K &key, V &value) override;
  bool remove(const K &key) override;

  size_t size() const;

private:
  std::list<std::pair<K, V>> caches_;

  mutable std::mutex mutex_;
};
