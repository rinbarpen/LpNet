#pragma once
#include "marcos.h"
#include "net/NetAddress.h"
#include "net/socket_util.h"

using SockfdAddressPair = std::pair<sockfd_t, NetAddress>;
// using SockfdAddressIpv6Tuple = std::tuple<sockfd_t, NetAddress, bool>;

struct NetDomain;
struct NetProtocol;

class Socket
{
public:
  SHARED_PTR_USING(Socket, ptr);

  Socket() = default;
  Socket(NetProtocol protocol);
  virtual ~Socket();

  /**
   * \brief warp the socket_api::bind
   * \param[in] addr 
   */
  int bind(const NetAddress &addr);
  /**
   * \brief warp the socket_api::listen
   * \param[in] backlog
   */
  int listen(int backlog);
  /**
   * \brief warp the socket_api::connect, use NetAddress instead of the raw data
   * \param[in] addr 
   */
  int connect(const NetAddress &addr);
  /**
   * \brief warp the socket_api::accept
   * \return the accepted socket
   */
  SockfdAddressPair accept();

  int close();

  int send(const void *data, int len);
  int recv(void *data, int len);

  SockfdAddressPair getSockfdAddressPair() const { return {sockfd_, addr_}; }
  sockfd_t getSockfd() const { return sockfd_; }
  bool isIpv6() const { return domain_.isIpv6(); }
  NetDomain getDomain() const { return domain_; }
  std::string getDomainName() const { return NetDomain::toString(domain_); }
  NetProtocol getProtocol() const { return protocol_; }
  std::string getProtocolName() const { return NetProtocol::toString(protocol_); }
  bool isValid() const { return sockfd_ > 0; }

  std::string getIp() const { return addr_.ip; }
  uint16_t getPort() const { return addr_.port; }
  NetAddress getIpPortPair() const { return addr_; }

  virtual std::string type() const { return "Socket"; }
protected:
  sockfd_t sockfd_{ kInvalidSocket };
  NetAddress addr_;
  NetDomain domain_;
  NetProtocol protocol_;

  // RawSocket raw_socket_;
};
