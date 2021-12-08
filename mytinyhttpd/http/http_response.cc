#include "mytinyhttpd/http/http_response.h"

#include <stdio.h>
#include <cstddef>

#include "mytinyhttpd/net/buffer.h"

namespace mytinyhttpd {

namespace net {

void HttpResponse::SetStatusLineAndAppend(HttpStatusCode code, Slice message) {
  assert((state_ == kExpectStatusLine) && (state_ = kExpectHeaderOrBody));
  char buf[32];
  snprintf(buf, sizeof buf, "HTTP/1.1 %d ", code);
  buffer_.Append(buf);
  buffer_.Append(message);
  buffer_.Append("\r\n");
}

void HttpResponse::SetCloseConnectionAndAppend(bool on) {
  assert(state_ == kExpectHeaderOrBody);
  if (is_close_connection_) {
    buffer_.Append("Connection: close\r\n");
  } else {
    buffer_.Append("Connection: Keep-Alive\r\n");
  }
}

void HttpResponse::AddHeader(Slice key, Slice value) {
  assert(state_ == kExpectHeaderOrBody);
  buffer_.Append(key);
  buffer_.Append(": ");
  buffer_.Append(value);
  buffer_.Append("\r\n");
}

void HttpResponse::AppendContentLength(Slice body) {
  char buf[32];
  snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body.size());
  buffer_.Append(buf);
  buffer_.Append("\r\n");
}

void HttpResponse::AppendContentLength(size_t len) {
  char buf[32];
  snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", len);
  buffer_.Append(buf);
  buffer_.Append("\r\n");
}

void HttpResponse::SetBodyAndAppend(Slice body) {
  assert((state_ == kExpectHeaderOrBody) && (state_ = kAppendAll));
  AppendContentLength(body);

  buffer_.Append(body);
}

void HttpResponse::SetBodyAndAppend(const unsigned char* body, size_t len) {
  assert((state_ == kExpectHeaderOrBody) && (state_ = kAppendAll));
  AppendContentLength(len);

  buffer_.Append(body, len);
}

}  // namespace net

}  // namespace mytinyhttpd