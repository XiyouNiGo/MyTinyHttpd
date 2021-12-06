#include "mytinyhttpd/http/http_server.h"

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/http/http_context.h"
#include "mytinyhttpd/http/http_request.h"
#include "mytinyhttpd/http/http_response.h"

namespace mytinyhttpd {

namespace net {

namespace detail {

void DefaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->SetStatusCode(HttpResponse::k404NotFound);
  resp->SetStatusMessage("Not Found");
  resp->SetCloseConnection(true);
}

}  // namespace detail

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listen_addr,
                       const std::string& name, TcpServer::Option option)
    : server_(loop, listen_addr, name, option),
      http_callback_(detail::DefaultHttpCallback) {
  server_.SetConnectionCallback(std::bind(&HttpServer::OnConnection, this, _1));
  server_.SetMessageCallback(
      std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
}

void HttpServer::Start() {
  LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on "
           << server_.ip_port();
  server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    conn->SetContext(HttpContext());
  }
}

void HttpServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receive_time) {
  HttpContext* context = AnyCast<HttpContext>(conn->mutable_context());

  if (!context->ParseRequest(buf, receive_time)) {
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }

  if (context->IsGotAll()) {
    OnRequest(conn, context->request());
    context->Reset();
  }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req) {
  Slice connection = req.GetHeader("Connection");
  bool close =
      connection == "close" ||
      (req.version() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  http_callback_(req, &response);
  Buffer buf;
  response.AppendToBuffer(&buf);
  conn->Send(&buf);
  if (response.IsCloseConnection()) {
    conn->Shutdown();
  }
}

}  // namespace net

}  // namespace mytinyhttpd