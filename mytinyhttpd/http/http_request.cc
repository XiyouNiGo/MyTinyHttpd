#include "mytinyhttpd/http/http_request.h"

namespace mytinyhttpd {

namespace net {

const std::string HttpRequest::ToDebugString() {
  std::string debug_info;
  debug_info.append(receive_time().ToFormattedString());
  debug_info.append(": receive a http request\n");
  debug_info.append(ToMethodString());
  debug_info.append(" ");
  debug_info.append(path());
  debug_info.append(" ");
  debug_info.append(query());
  debug_info.append("\n");

  for (auto &header : headers_) {
    debug_info.append(header.first);
    debug_info.append(": ");
    debug_info.append(header.second);
    debug_info.append("\n");
  }
  return debug_info;
}

}  // namespace net

}  // namespace mytinyhttpd
