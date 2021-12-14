#ifndef MYTINYHTTPD_HTTP_HTTP_SERVER_H_
#define MYTINYHTTPD_HTTP_HTTP_SERVER_H_

#include <cstdint>
#include <string>
#include <unordered_map>

#include "mytinyhttpd/net/tcp_server.h"
#include "mytinyhttpd/net/timing_wheel.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/slice.h"

namespace mytinyhttpd {

namespace net {

extern unsigned char favicon[3086];

class HttpRequest;
class HttpResponse;

class HttpServerConfig : public copyable {
 public:
  HttpServerConfig()
      : is_valid_(false), port_(default_port_), option_(default_option_) {}
  HttpServerConfig(Slice filename);
  ~HttpServerConfig() = default;

  bool IsValid() const { return is_valid_; }

  uint16_t port() const { return port_; }
  void SetPort(uint16_t port) { port_ = port; }

  const std::string& domain() const { return domain_; }
  void SetDomain(const std::string& domain) { domain_ = domain; }

  const std::string& docroot() const { return docroot_; }
  void SetDocroot(const std::string& docroot) { docroot_ = docroot; }

  TcpServer::Option option() const { return option_; }
  void option(TcpServer::Option option) { option_ = option; }

  bool ReplaceIfNotDefault(const HttpServerConfig& config);

  std::string GetRealPath(const std::string& path) {
    return docroot() + (*(docroot().cend() - 1) != '/' ? "/" : "") + path;
  }

  bool is_valid_;

  uint16_t port_;
  std::string domain_;
  std::string docroot_;
  TcpServer::Option option_;

  static const std::string default_domain_;
  static const std::string default_docroot_;
  static const std::string default_conf_file_;
  static const uint16_t default_port_;
  static const TcpServer::Option default_option_;
};

class HttpServer : public noncopyable {
 public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const std::string& name,
             HttpServerConfig* config, int idle_seconds = 8);

  EventLoop* loop() const { return server_.loop(); }

  void SetHttpCallback(const HttpCallback& cb) { http_callback_ = cb; }

  void SetThreadNum(int num_threads) { server_.SetThreadNum(num_threads); }

  void Start();

  static const char* GetMimeType(const std::string& real_path);

 private:
  void OnConnection(const TcpConnectionPtr& conn);
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receive_time);
  void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

  void DefaultHttpCallback(const HttpRequest& req, HttpResponse* resp);

  TcpServer server_;
  HttpServerConfig* config_;
  HttpCallback http_callback_;

  TimingWheel timing_wheel_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_SERVER_H_
