#ifndef MYTINYHTTPD_HTTP_HTTP_CONTEXT_H_
#define MYTINYHTTPD_HTTP_HTTP_CONTEXT_H_

#include "mytinyhttpd/http/http_request.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class Buffer;

class HttpContext : public copyable {
 public:
  enum HttpRequestParseState : char {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
    kNumHttpRequestParseState
  };
  static_assert(static_cast<int>(kNumHttpRequestParseState) <= (1 << 7),
                "HttpContext::HttpRequestParseState overflow up");

  HttpContext() : state_(kExpectRequestLine) {}
  ~HttpContext() = default;

  bool ParseRequest(Buffer* buf, Timestamp receive_time);

  bool IsGotAll() const { return state_ == kGotAll; }

  void Reset() {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const { return request_; }

  HttpRequest& request() { return request_; }

 private:
  bool ProcessRequestLine(const char* begin, const char* end);

  HttpRequestParseState state_;
  HttpRequest request_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_CONTEXT_H_
