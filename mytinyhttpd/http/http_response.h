#ifndef MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_
#define MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_

#include <map>

#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

class Buffer;

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
      : status_code_(kUnknown), is_close_connection_(is_close) {}

  void SetStatusCode(HttpStatusCode code) { status_code_ = code; }

  void SetStatusMessage(const std::string& message) {
    status_message_ = message;
  }

  void SetCloseConnection(bool on) { is_close_connection_ = on; }

  bool IsCloseConnection() const { return is_close_connection_; }

  void SetContentType(const std::string& contentType) {
    AddHeader("Content-Type", contentType);
  }

  void AddHeader(Slice key, Slice value) {
    headers_[key.ToString()] = value.ToString();
  }

  void SetBody(const std::string& body) { body_ = body; }

  void AppendToBuffer(Buffer* output) const;

 private:
  std::map<std::string, std::string> headers_;
  HttpStatusCode status_code_;
  std::string status_message_;
  bool is_close_connection_;
  std::string body_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_RESPONSE_H_
