#include "mytinyhttpd/net/timing_list.h"

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/tcp_connnection.h"

namespace mytinyhttpd {

namespace net {

void TimingList::OnTimer() {
  Timestamp now = Timestamp::Now();
  for (WeakConnectionList::iterator it = connection_list_.begin();
       it != connection_list_.end();) {
    TcpConnectionPtr conn = it->lock();
    if (conn) {
      Node* node = AnyCast<Node>(conn->mutable_timing_context());
      double age = static_cast<double>(now - node->last_receive_time);
      if (age > idle_seconds_) {
        if (conn->IsConnected()) {
          conn->Shutdown();
          LOG_INFO << "shutting down " << conn->name();
          conn->ForceCloseWithDelay(3.5);
        }
      } else if (age < 0) {
        LOG_WARN << "Time jump";
        node->last_receive_time = now;
      } else {
        break;
      }
      ++it;
    } else {
      LOG_WARN << "Expired";
      it = connection_list_.erase(it);
    }
  }
}

void TimingList::OnConnection(const TcpConnectionPtr& conn) {
  if (conn->IsConnected()) {
    Node node;
    node.last_receive_time = Timestamp::Now();
    connection_list_.push_back(conn);
    node.position = --connection_list_.end();
    conn->SetTimingContext(node);
  } else {
    assert(!conn->timing_context().empty());
    Node* node = AnyCast<Node>(conn->mutable_timing_context());
    connection_list_.erase(node->position);
  }
}

void TimingList::OnMessage(const TcpConnectionPtr& conn,
                           Timestamp receive_time) {
  assert(!conn->timing_context().empty());
  Node* node = AnyCast<Node>(conn->mutable_timing_context());
  node->last_receive_time = receive_time;
  connection_list_.splice(connection_list_.end(), connection_list_,
                          node->position);
  assert(node->position == --connection_list_.end());
}

}  // namespace net

}  // namespace mytinyhttpd