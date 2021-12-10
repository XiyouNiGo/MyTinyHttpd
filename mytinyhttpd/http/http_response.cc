#include "mytinyhttpd/http/http_response.h"

#include <stdio.h>

#include <cstddef>

#include "mytinyhttpd/net/buffer.h"
#include "mytinyhttpd/utils/file.h"

namespace mytinyhttpd {

namespace net {

void HttpResponse::AppendStatusLine(HttpStatusCode code, Slice message) {
  assert(state_ == kAppendNothing);
  state_ = kAppendStatusLine;
  char buf[32];
  snprintf(buf, sizeof buf, "%s %d ", GetHttpVersion(), code);
  buffer_.Append(buf);
  buffer_.Append(message);
  buffer_.Append("\r\n");
}

void HttpResponse::AppendCloseConnection(bool on) {
  assert(state_ == kAppendStatusLine || state_ == kAppendHeader);
  state_ = kAppendHeader;
  is_close_connection_ = on;
  if (is_close_connection_) {
    buffer_.Append("Connection: close\r\n");
  } else {
    buffer_.Append("Connection: Keep-Alive\r\n");
  }
}

void HttpResponse::AppendHeader(Slice key, Slice value) {
  assert(state_ == kAppendStatusLine || state_ == kAppendHeader);
  state_ = kAppendHeader;
  buffer_.Append(key);
  buffer_.Append(": ");
  buffer_.Append(value);
  buffer_.Append("\r\n");
}

void HttpResponse::AppendContentLength(size_t len) {
  assert(state_ == kAppendStatusLine || state_ == kAppendHeader);
  state_ = kAppendHeader;
  char buf[32];
  snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", len);
  buffer_.Append(buf);
}

void HttpResponse::AppendBody(Slice body) {
  assert(state_ == kAppendHeader);
  AppendContentLength(body.size());
  AppendHeadersEnd();
  assert(state_ == kAppendHeadersEnd);
  state_ = kAppendBody;
  buffer_.Append(body);
}

void HttpResponse::AppendBody(const unsigned char* body, size_t len) {
  assert(state_ == kAppendHeader);
  AppendContentLength(len);
  AppendHeadersEnd();
  assert(state_ == kAppendHeadersEnd);
  state_ = kAppendBody;
  buffer_.Append(body, len);
}

void HttpResponse::AppendBody(std::ifstream& is) {
  long len = GetFileSize(is);
  assert(state_ == kAppendHeader);
  AppendContentLength(len);
  AppendHeadersEnd();
  assert(state_ == kAppendHeadersEnd);
  state_ = kAppendBody;
  buffer_.Append(is, len);
}

}  // namespace net

}  // namespace mytinyhttpd