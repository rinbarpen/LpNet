#pragma once
#include "Mutex.h"
#include "net/socket_util.h"

class Buffer
{
protected:
	static constexpr int kMaxBytesPerRead = 4096;
public:
	explicit Buffer(size_t capacity);
	~Buffer();

  /**
   * \brief read buffer to s
   * \param[out] s 
   * \param[in] n 
   * \return the bytes we read actually
   */
  size_t read(char *s, size_t n);
	/**
   * \brief write buffer from s
	 * \param[in] s
	 * \param[in] n
   * \return the bytes we write actually
   */
	size_t write(const char *s, size_t n);
  /**
   * \brief read from fd
   * \param fd 
   * \return the bytes we read actually
   */
  int read(sockfd_t fd);
	/**
   * \brief write to fd
   * \param fd
   * \return the bytes we write actually
   */
	int write(sockfd_t fd, size_t size, int ms_timeout = 0);

	void reset(const size_t newCapacity);

	bool full() const { Mutex::lock locker(mutex_); return fullInternal(); }
	bool empty() const { Mutex::lock locker(mutex_); return emptyInternal(); }
	size_t capacity() const { Mutex::lock locker(mutex_); return capacityInternal(); }
	size_t size() const { Mutex::lock locker(mutex_); return sizeInternal(); }

	size_t readableBytes() const { Mutex::lock locker(mutex_); return sizeInternal(); }
	size_t writableBytes() const { Mutex::lock locker(mutex_); return capacity_ - sizeInternal(); }

private:
	inline size_t indexOf(const size_t pos, const size_t n) const { return (pos + n) % (capacity_ + 1); }
	inline bool fullInternal() const { return sizeInternal() == capacity_; }
	inline bool emptyInternal() const { return put_pos_ == get_pos_; }
	inline size_t sizeInternal() const { return (capacity_ + put_pos_ - get_pos_) % (capacity_ + 1); }
	inline size_t capacityInternal() const { return capacity_; }

private:
	size_t capacity_{ 0 };
	size_t put_pos_{ 0 };
	size_t get_pos_{ 0 };
	char *data_;

	mutable Mutex::type mutex_;
};
