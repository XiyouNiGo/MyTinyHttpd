#ifndef MYTINYHTTPD_HTTP_HTTP_REQUEST_H_
#define MYTINYHTTPD_HTTP_HTTP_REQUEST_H_

#include <assert.h>
#include <stdio.h>

#include <cstring>
#include <map>

#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/slice.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class HttpRequest : public copyable {
 public:
  enum Method : char {
    kInvalid,
    kGet,
    kPost,
    kHead,
    kPut,
    kDelete,
    kTrack,
    kOptions,
    kNumHttpRequestMethod
  };
  static_assert(static_cast<int>(kNumHttpRequestMethod) <= (1 << 7),
                "HttpRequest::Method overflow up");

  enum Version : char { kUnknown, kHttp10, kHttp11, kNumHttpRequestVersion };
  static_assert(static_cast<int>(kNumHttpRequestVersion) <= (1 << 7),
                "HttpRequest::Version overflow up");

  HttpRequest() : method_(kInvalid), version_(kUnknown) {}

  void SetVersion(Version v) { version_ = v; }

  Version version() const { return version_; }

  void SetMethod(const char* start, const char* end) {
    assert(method_ == kInvalid);
    if (::memcmp(start, "GET", 3) == 0) {
      method_ = kGet;
    } else if (::memcmp(start, "POST", 4) == 0) {
      method_ = kPost;
    } else if (::memcmp(start, "HEAD", 4) == 0) {
      method_ = kHead;
    } else if (::memcmp(start, "PUT", 3) == 0) {
      method_ = kPut;
    } else if (::memcmp(start, "DELETE", 6) == 0) {
      method_ = kDelete;
      // } else if (::memcmp(start, "TRACK", 6) == 0) {
      //   method_ = kTrack;
    } else if (::memcmp(start, "OPTIONS", 7) == 0) {
      method_ = kOptions;
    } else {
      method_ = kInvalid;
    }
  }

  Method method() const { return method_; }

  const char* ToMethodString() const { return methods_map[method_]; }

  const std::string ToDebugString();

  void SetPath(const char* start, const char* end) { path_.assign(start, end); }

  const std::string& path() const { return path_; }

  void SetQuery(const char* start, const char* end) {
    query_.assign(start, end);
  }

  const std::string& query() const { return query_; }

  void SetReceiveTime(Timestamp t) { receive_time_ = t; }

  Timestamp receive_time() const { return receive_time_; }

  void SetBody(const char* start, const char* end) { body_.assign(start, end); }

  const std::string& body() const { return body_; }

  void AddHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
      ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
      value.resize(value.size() - 1);
    }
    headers_[field] = value;
  }

  Slice GetHeader(const std::string& field) const {
    Slice result;
    std::map<std::string, std::string>::const_iterator it =
        headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }

  const std::map<std::string, std::string>& headers() const { return headers_; }

  void swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    receive_time_.swap(that.receive_time_);
    headers_.swap(that.headers_);
  }

 private:
  const char* methods_map[kNumHttpRequestMethod] = {
      "UNKNOWN", "GET", "POST", "HEAD", "PUT", "DELETE", "TRACK", "OPTIONS"};

  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  Timestamp receive_time_;
  std::map<std::string, std::string> headers_;
  std::string body_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_REQUEST_H_
