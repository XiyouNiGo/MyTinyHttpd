#include "mytinyhttpd/http/http_context.h"
#include <iostream>

#include "mytinyhttpd/net/buffer.h"

namespace mytinyhttpd {

namespace net {

bool HttpContext::ProcessRequestLine(const char* begin, const char* end) {
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.SetMethod(start, space)) {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end) {
      const char* question = std::find(start, space, '?');
      if (question != space) {
        request_.SetPath(start + 1, question);
        request_.SetQuery(question, space);
      } else {
        request_.SetPath(start + 1, space);
      }
      start = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
      if (succeed) {
        if (*(end - 1) == '1') {
          request_.SetVersion(HttpRequest::kHttp11);
        } else if (*(end - 1) == '0') {
          request_.SetVersion(HttpRequest::kHttp10);
        } else {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

bool HttpContext::ParseRequest(Buffer* buf, Timestamp receive_time) {
  bool ok = true;
  bool has_more = true;
  while (has_more) {
    if (state_ == kExpectRequestLine) {
      const char* crlf = buf->FindCrlf();
      if (crlf) {
        ok = ProcessRequestLine(buf->Peek(), crlf);
        if (ok) {
          request_.SetReceiveTime(receive_time);
          buf->RetrieveUntil(crlf + 2);
          state_ = kExpectHeaders;
        } else {
          has_more = false;
        }
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectHeaders) {
      const char* crlf = buf->FindCrlf();
      if (crlf) {
        const char* colon = std::find(buf->Peek(), crlf, ':');
        if (colon != crlf) {
          request_.AddHeader(buf->Peek(), colon, crlf);
        } else {
          state_ = kGotAll;
          LOG_DEBUG << request_.ToDebugString();
          has_more = false;
        }
        buf->RetrieveUntil(crlf + 2);
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectBody) {
      // TODO: parse body
    }
  }
  return ok;
}

}  // namespace net

}  // namespace mytinyhttpd