#ifndef MYTINYHTTPD_HTTP_HTTP_SERVER_H_
#define MYTINYHTTPD_HTTP_HTTP_SERVER_H_

#include "mytinyhttpd/net/tcp_server.h"
#include "mytinyhttpd/utils/noncopyable.h"

namespace mytinyhttpd {

namespace net {

class HttpRequest;
class HttpResponse;

class HttpServer : public noncopyable {
 public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listen_addr,
             const std::string& name,
             TcpServer::Option option = TcpServer::kNoReusePort);

  EventLoop* loop() const { return server_.loop(); }

  void SetHttpCallback(const HttpCallback& cb) { http_callback_ = cb; }

  void SetThreadNum(int num_threads) { server_.SetThreadNum(num_threads); }

  void Start();

 private:
  void OnConnection(const TcpConnectionPtr& conn);
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receive_time);
  void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback http_callback_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_HTTP_HTTP_SERVER_H_
