#include "mytinyhttpd/http/http_server.h"

#include <gtest/gtest.h>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/http/http_request.h"
#include "mytinyhttpd/http/http_response.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/timestamp.h"

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
  std::cout << "Headers " << req.ToMethodString() << " " << req.path()
            << std::endl;

  const std::map<std::string, std::string>& headers = req.headers();
  for (const auto& header : headers) {
    std::cout << header.first << ": " << header.second << std::endl;
  }

  if (req.path() == "/") {
    resp->SetStatusCode(HttpResponse::k200Ok);
    resp->SetStatusMessage("OK");
    resp->SetContentType("text/html");
    resp->AddHeader("Server", "MyTinyHttpd");
    std::string now = Timestamp::Now().ToFormattedString();
    resp->SetBody(
        "<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " +
        now + "</body></html>");
  } else if (req.path() == "/hello") {
    resp->SetStatusCode(HttpResponse::k200Ok);
    resp->SetStatusMessage("OK");
    resp->SetContentType("text/plain");
    resp->AddHeader("Server", "MyTinyHttpd");
    resp->SetBody("hello, world!\n");
  } else {
    resp->SetStatusCode(HttpResponse::k404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
  }
}

TEST(HttpServerTest, AllTest) {
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "HttpServer");
  server.SetHttpCallback(OnRequest);
  server.SetThreadNum(8);
  server.Start();
  loop.Loop();
}