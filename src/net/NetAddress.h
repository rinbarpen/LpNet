#pragma once
#include <string>
#include <unordered_set>
#include <random>

#include "net/net.h"

#include "Mutex.h"
#include "Singleton.h"

struct NetAddress
{
  std::string ip;
  uint16_t port;
};

class MulticastAddr
{
public:
	std::string getAddr()
	{
		// TODO: Add ipv6
		Mutex::lock locker(mutex_);
		std::string addr_str;
		struct sockaddr_in addr = { 0 };
		std::random_device rd;

		for (int n = 0; n <= 10; n++) {
			uint32_t range = 0xE8FFFFFF - 0xE8000100;
			addr.sin_addr.s_addr = htonl(0xE8000100 + (rd()) % range);
			// Note:
		  addr_str.reserve(INET_ADDRSTRLEN);
		  inet_ntop(AF_INET, (struct sockaddr*)&addr, addr_str.data(), INET_ADDRSTRLEN);

			if (addrs_.find(addr_str) != addrs_.end()) {
				addr_str.clear();
			}
			else {
				addrs_.insert(addr_str);
				break;
			}
		}

		return addr_str;
	}

	void release(const std::string &addr) {
		Mutex::lock locker(mutex_);
		addrs_.erase(addr);
	}

private:
	Mutex::type mutex_;
	std::unordered_set<std::string> addrs_;
};

using SingleMulticastAddr = Singleton<MulticastAddr>;
