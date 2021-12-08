#include "mytinyhttpd/http/http_server.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "mytinyhttpd/base/logging.h"
#include "mytinyhttpd/http/http_context.h"
#include "mytinyhttpd/http/http_request.h"
#include "mytinyhttpd/http/http_response.h"

using namespace nlohmann;

namespace mytinyhttpd {

namespace net {

namespace detail {

void DefaultHttpCallback(const HttpRequest& req, HttpResponse* resp) {
  if (req.method() == HttpRequest::kGet) {
  } else if (req.method() == HttpRequest::kPost) {
  } else if (req.method() == HttpRequest::kHead) {
  } else if (req.method() == HttpRequest::kPut) {
  } else if (req.method() == HttpRequest::kDelete) {
  } else if (req.method() == HttpRequest::kTrack) {
  } else if (req.method() == HttpRequest::kOptions) {
  } else {
    resp->SetStatusLineAndAppend(HttpResponse::k404NotFound, "Not Found");
    resp->SetCloseConnectionAndAppend(true);
  }
}
}  // namespace detail

HttpServerConfig::HttpServerConfig(Slice filename) : is_valid(false) {
  std::ifstream config_file(filename.data());
  if (config_file) {
    try {
      json config_json;
      config_file >> config_json;
      assert(config_json.is_object());
      LOG_WARN << "HttpServerConfig: " << config_json.dump();
      if (config_json.contains("domain")) {
        domain_ = config_json["domain"];
      }
      if (config_json.contains("docroot")) {
        docroot_ = config_json["docroot"];
      }
      is_valid = true;
    } catch (...) {
      LOG_FATAL << "HttpServerConfig file parse error";
    }
  }
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listen_addr,
                       const std::string& name, TcpServer::Option option,
                       Slice filename)
    : server_(loop, listen_addr, name, option),
      http_callback_(detail::DefaultHttpCallback),
      config(filename) {
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
  conn->Send(&response.buffer());
  if (response.IsCloseConnection()) {
    conn->Shutdown();
  }
}

}  // namespace net

}  // namespace mytinyhttpd