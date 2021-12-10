#include "mytinyhttpd/mytinyhttpd/mytinyhttpd.h"

#include "mytinyhttpd/http/http_server.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

int main(int argc, char *argv[]) {
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "MyTinyHttpd");
  server.SetThreadNum(8);
  server.Start();
  loop.Loop();
  return 0;
}