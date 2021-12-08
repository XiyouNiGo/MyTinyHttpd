#include "mytinyhttpd/http/http_server.h"

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include <fstream>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/http/http_request.h"
#include "mytinyhttpd/http/http_response.h"
#include "mytinyhttpd/net/event_loop.h"
#include "mytinyhttpd/net/inet_address.h"
#include "mytinyhttpd/utils/timestamp.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;
using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

TEST(HttpServerTest, HttpServerConfigTest) {
  {
    HttpServerConfig config("not_exist_file");
    ASSERT_TRUE(config.domain().empty());
    ASSERT_TRUE(config.docroot().empty());
    ASSERT_FALSE(config.IsValid());
  }
  {
    std::ofstream o("exist_file");
    json j = json::parse(R"({"domain": "www.baidu.com", "docroot": "/"})");
    o << j << std::endl;
    HttpServerConfig config("exist_file");
    ASSERT_EQ(config.domain(), "www.baidu.com");
    ASSERT_EQ(config.docroot(), "/");
    ASSERT_TRUE(config.IsValid());
  }
}

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
  std::cout << "Headers " << req.ToMethodString() << " " << req.path()
            << std::endl;

  const std::map<std::string, std::string>& headers = req.headers();
  for (const auto& header : headers) {
    std::cout << header.first << ": " << header.second << std::endl;
  }

  if (req.path() == "/") {
    resp->SetStatusLineAndAppend(HttpResponse::k200Ok, "OK");
    resp->SetContentTypeAndAppend("text/html");
    resp->AddHeader("Server", "MyTinyHttpd");
    std::string now = Timestamp::Now().ToFormattedString();
    resp->SetBodyAndAppend(
        "<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " +
        now + "</body></html>");
  } else if (req.path() == "/hello") {
    resp->SetStatusLineAndAppend(HttpResponse::k200Ok, "OK");
    resp->SetContentTypeAndAppend("text/plain");
    resp->AddHeader("Server", "MyTinyHttpd");
    resp->SetBodyAndAppend("hello, world!\n");
  } else {
    resp->SetStatusLineAndAppend(HttpResponse::k404NotFound, "Not Found");
    resp->SetCloseConnectionAndAppend(true);
  }
}

TEST(HttpServerTest, HttpServerTest) {
  EventLoop loop;
  HttpServer server(&loop, InetAddress(8000), "HttpServer");
  server.SetHttpCallback(OnRequest);
  server.SetThreadNum(8);
  server.Start();
  loop.Loop();
}