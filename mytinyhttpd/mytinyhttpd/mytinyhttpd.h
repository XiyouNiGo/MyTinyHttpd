#ifndef MYTINYHTTPD_HTTPD_MYTINYHTTPD_H_
#define MYTINYHTTPD_HTTPD_MYTINYHTTPD_H_

#include "mytinyhttpd/http/http_server.h"
#include "mytinyhttpd/utils/program_options.h"

namespace mytinyhttpd {

namespace net {

class MyTinyHttpd {
 public:
  static void VerifyOptions(const boost::program_options::variables_map& vm);

  static const char* op_desc() { return op_desc_; }
  static const char* version_desc() { return version_desc_; }

 private:
  static const char* op_desc_;
  static const char* version_desc_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTPD_MYTINYHTTPD_H_