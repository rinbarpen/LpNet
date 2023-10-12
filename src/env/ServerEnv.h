#pragma once
#include "Env.h"
#include "net/NetAddress.h"
#include "net/Socket.h"

class ServerEnv : public Env
{
public:
  virtual ~ServerEnv() = default;
  virtual std::string toYamlString() const
  { 
    YAML::Node node, args;
    args["type"] = type();
    args["name"] = server_name_;
    args["max backlog"] = max_backlog_;
    args["max session num"] = max_session_num_;
    args["max upload speed rate per client"] = max_upload_speed_rate_per_cli_;
    args["max download speed rate per client"] = max_download_speed_rate_per_cli_;
    args["max buffer size"] = max_buffer_size_;

    node[server_name_] = args;

    std::stringstream ss;
    ss << node;
    return ss.str();
  }

  virtual std::string type() const { return "Server"; }
  /**
   * a socket binds an ip and port
   * protocol my used
   *   - tcp|udp|http (multi-protocols)
   * max backlog
   * max sessions
   * max upload speed rate per cli
   * max download speed rate per cli
   * max buffer size
   *
   */
  // Socket sock_;     /* socket fd and ip, port */
  // std::vector<std::string> protocols_;
  int max_backlog_;
  int max_session_num_;
  int max_upload_speed_rate_per_cli_;
  int max_download_speed_rate_per_cli_;
  int max_buffer_size_;
  std::string server_name_;
};

class TcpServerEnv : public ServerEnv
{
public:
  virtual ~TcpServerEnv() = default;
  virtual std::string toYamlString() const override { return ""; }
  virtual std::string type() const override { return "TcpServer"; }
private:

};
class UdpServerEnv : public ServerEnv
{
public:
  virtual ~UdpServerEnv() = default;
  virtual std::string toYamlString() const override { return ""; }
  virtual std::string type() const override { return "UdpServer"; }
private:
  
};
