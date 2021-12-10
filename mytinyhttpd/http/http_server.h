#ifndef MYTINYHTTPD_HTTP_HTTP_SERVER_H_
#define MYTINYHTTPD_HTTP_HTTP_SERVER_H_

#include <string>
#include <unordered_map>

#include "mytinyhttpd/net/tcp_server.h"
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
  HttpServerConfig(Slice filename = "mytinyhttpd.config");
  ~HttpServerConfig() = default;

  bool IsValid() { return is_valid; }

  const std::string& domain() { return domain_; }
  const std::string& docroot() { return docroot_; }

 private:
  bool is_valid;
  std::string domain_;
  std::string docroot_;
};

class HttpServer : public noncopyable {
 public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listen_addr,
             const std::string& name,
             TcpServer::Option option = TcpServer::kNoReusePort,
             Slice filename = "mytinyhttpd.config");

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
  HttpServerConfig config_;
  HttpCallback http_callback_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_SERVER_H_
