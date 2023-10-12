#pragma once

#include <iostream>
#include <net/tcp/TcpServer.h>

#include "net/EpollManager.h"
#include "net/EventLoop.h"
#include "net/SelectManager.h"

int main()
{
#if defined(__linux) || defined(__linux__) 
  signal(SIGPIPE, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGUSR1, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGKILL, SIG_IGN);
#endif

  ThreadPool pool(4);
  EventLoop eventLoop(pool);
  eventLoop.addTaskScheduler(SelectManager::make_shared("TcpServer"));
  // TcpServerEnv env;
  // env.max_backlog_ = 10;
  TcpServer server(&eventLoop);

  server.start("127.0.0.1", 12345, 10);

  eventLoop.run();

  server.stop();

  return 0;
}
