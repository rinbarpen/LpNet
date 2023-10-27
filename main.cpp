#pragma once

#include <iostream>
#include <net/tcp/TcpServer.h>

#include "net/EpollManager.h"
#include "net/EventLoop.h"
#include "net/SelectManager.h"
#include "platform.h"

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 15777

int main()
{
// #if defined(__LINUX__)
//   signal(SIGPIPE, SIG_IGN);
//   signal(SIGQUIT, SIG_IGN);
//   signal(SIGUSR1, SIG_IGN);
//   signal(SIGTERM, SIG_IGN);
//   signal(SIGKILL, SIG_IGN);
// #elif defined(__WIN__)
//   WSADATA wsaData;
//   int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
//   if (r != NO_ERROR) {
//     wprintf(L"Error at WSAStartup()\n");
//     return 1;
//   }
// #endif

  RECORD_ON();

  ThreadPool pool(4);
  EventLoop eventLoop(pool);
  eventLoop.addTaskScheduler(SelectManager::make_shared("TcpServer"));
  // TcpServerEnv env;
  // env.max_backlog_ = 10;
  TcpServer server(&eventLoop);

  server.start(SERVER_HOST, SERVER_PORT, 10);

  server.doEventLoop();

  server.stop();

  RECORD_OFF();
// #if defined(__WIN__)
//   WSACleanup();
// #endif
  return 0;
}
