#include <gtest/gtest.h>

#include "mytinyhttpd/http/http_context.h"
#include "mytinyhttpd/net/buffer.h"
#include "mytinyhttpd/utils/timestamp.h"

using namespace mytinyhttpd;
using namespace mytinyhttpd::net;

TEST(HttpRequestTest, ParseRequestAllInOneTest) {
  HttpContext context;
  Buffer input;
  input.Append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.baidu.com\r\n"
      "\r\n");

  ASSERT_TRUE(context.ParseRequest(&input, Timestamp::Now()));
  ASSERT_TRUE(context.IsGotAll());
  const HttpRequest& request = context.request();
  ASSERT_EQ(request.method(), HttpRequest::kGet);
  ASSERT_EQ(request.path(), std::string("/index.html"));
  ASSERT_EQ(request.version(), HttpRequest::kHttp11);
  ASSERT_EQ(request.GetHeader("Host"), std::string("www.baidu.com"));
  ASSERT_EQ(request.GetHeader("User-Agent"), std::string(""));
}

TEST(HttpRequestTest, ParseRequestInTwoPiecesTest) {
  std::string all(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.baidu.com\r\n"
      "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1) {
    HttpContext context;
    Buffer input;
    input.Append(all.c_str(), sz1);
    ASSERT_TRUE(context.ParseRequest(&input, Timestamp::Now()));
    ASSERT_TRUE(!context.IsGotAll());

    size_t sz2 = all.size() - sz1;
    input.Append(all.c_str() + sz1, sz2);
    ASSERT_TRUE(context.ParseRequest(&input, Timestamp::Now()));
    ASSERT_TRUE(context.IsGotAll());
    const HttpRequest& request = context.request();
    ASSERT_EQ(request.method(), HttpRequest::kGet);
    ASSERT_EQ(request.path(), std::string("/index.html"));
    ASSERT_EQ(request.version(), HttpRequest::kHttp11);
    ASSERT_EQ(request.GetHeader("Host"), std::string("www.baidu.com"));
    ASSERT_EQ(request.GetHeader("User-Agent"), std::string(""));
  }
}

TEST(HttpRequestTest, ParseRequestEmptyHeaderValueTest) {
  HttpContext context;
  Buffer input;
  input.Append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.baidu.com\r\n"
      "User-Agent:\r\n"
      "Accept-Encoding: \r\n"
      "\r\n");

  ASSERT_TRUE(context.ParseRequest(&input, Timestamp::Now()));
  ASSERT_TRUE(context.IsGotAll());
  const HttpRequest& request = context.request();
  ASSERT_EQ(request.method(), HttpRequest::kGet);
  ASSERT_EQ(request.path(), std::string("/index.html"));
  ASSERT_EQ(request.version(), HttpRequest::kHttp11);
  ASSERT_EQ(request.GetHeader("Host"), std::string("www.baidu.com"));
  ASSERT_EQ(request.GetHeader("User-Agent"), std::string(""));
  ASSERT_EQ(request.GetHeader("Accept-Encoding"), std::string(""));
}
