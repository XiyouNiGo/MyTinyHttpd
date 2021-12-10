#include "mytinyhttpd/http/http_server.h"

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include <fstream>
#include <string>

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

TEST(HttpServerTest, GetMimeTypeTest) {
  ASSERT_STREQ(HttpServer::GetMimeType("index.html"),
               "text/html; charset=utf-8");
  ASSERT_STREQ(HttpServer::GetMimeType(""), "application/octet-stream");
  ASSERT_STREQ(HttpServer::GetMimeType("no_suffix"),
               "application/octet-stream");
}

void OnRequest(const HttpRequest& req, HttpResponse* resp) {
  if (req.path().empty()) {
    resp->AppendStatusLine(HttpResponse::k200Ok, "OK");
    resp->AppendContentType("text/html");
    resp->AppendHeader("Server", "MyTinyHttpd");
    std::string now = Timestamp::Now().ToFormattedString();
    resp->AppendCloseConnection();
    resp->AppendBody(
        "<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " +
        now + "</body></html>");
  } else if (req.path() == "hello") {
    resp->AppendStatusLine(HttpResponse::k200Ok, "OK");
    resp->AppendContentType("text/plain");
    resp->AppendHeader("Server", "MyTinyHttpd");
    resp->AppendCloseConnection();
    resp->AppendBody("hello, world!\n");
  } else if (req.path() == "favicon.ico") {
    resp->AppendStatusLine(HttpResponse::k200Ok, "OK");
    resp->AppendContentType("image/png");
    resp->AppendCloseConnection();
    resp->AppendBody(favicon, sizeof favicon);
  } else {
    resp->AppendStatusLine(HttpResponse::k404NotFound, "Not Found");
    resp->AppendCloseConnection(true);
    resp->AppendHeadersEnd();
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