#ifndef MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_
#define MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_

#include <map>

#include "mytinyhttpd/net/buffer.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

class HttpResponse : public copyable {
 public:
  enum HttpStatusCode : int16_t {
    kUnknown,

    k100Continue = 100,
    k101SwitchingProtocols = 101,

    k200Ok = 200,
    k201Created = 201,
    k202Accepted = 202,
    k203NonAuthoritativeInformation = 203,
    k204NoContent = 204,
    k205ResetContent = 205,
    k206PartialContent = 206,

    k300MultipleChoices = 300,
    k301MovedPermanently = 301,
    k302Found = 302,
    k303SeeOther = 303,
    k304NotModified = 304,
    k305UseProxy = 305,
    k307TemporaryRedirect = 307,

    k400BadRequest = 400,
    k401Unauthorized = 401,
    k402PaymentRequired = 402,
    k403Forbidden = 403,
    k404NotFound = 404,
    k405MethodNotAllowd = 405,
    k406NotAcceptable = 406,
    k407ProxyAuthenticationRequired = 407,
    k408RequestTimeout = 408,
    k409Conflict = 409,
    k410Conflict = 410,
    k411LengthRequired = 411,
    k412PreconditionFailed = 412,
    k413RequestEntityTooLarge = 413,
    k414RequestUrlTooLong = 414,
    k415UnsupportedMediaType = 415,
    k416RequestedRangeNotSatisfiable = 416,
    k417ExpectationFailed = 417,

    k500InternalServerError = 500,
    k501NotImplemented = 501,
    k502BadGateway = 502,
    k503ServiceUnavailable = 503,
    k504GatewayTimeout = 504,
    k505HttpVersionNotSupported = 505
  };

  explicit HttpResponse(bool is_close)
      : state_(kExpectStatusLine), is_close_connection_(is_close) {}

  ~HttpResponse() { assert(state_ >= kExpectHeaderOrBody); }

  // Set***AndAppend must be called in order
  // the order:
  //  1. status line
  //  2. close connection or content type or headers
  //  3. body (if exists)
  void SetStatusLineAndAppend(HttpStatusCode code, Slice message);

  void SetCloseConnectionAndAppend(bool on);
  bool IsCloseConnection() const { return is_close_connection_; }

  void SetContentTypeAndAppend(Slice content_type) {
    AddHeader("Content-Type", content_type);
  }

  void AddHeader(Slice key, Slice value);

  // can be call once
  void SetBodyAndAppend(Slice body);

  Buffer& buffer() { return buffer_; }

 private:
  void AppendContentLength(Slice body);

  enum HttpResponseAppendState : char {
    kExpectStatusLine,
    kExpectHeaderOrBody,
    kAppendAll,
    kNumHttpResponseAppendState
  };
  static_assert(static_cast<int>(kNumHttpResponseAppendState) <= (1 << 7),
                "HttpResponse::HttpResponseAppendState overflow up");
  HttpResponseAppendState state_;

  bool is_close_connection_;
  Buffer buffer_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_
