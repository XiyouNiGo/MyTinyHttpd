#include "mytinyhttpd/net/timing_wheel.h"

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/tcp_connnection.h"

namespace mytinyhttpd {

namespace net {

TimingWheel::Entry::~Entry() {
  TcpConnectionPtr conn = weak_conn_.lock();
  if (conn) {
    conn->Shutdown();
  }
}

void TimingWheel::OnTimer() { connection_buckets_.push_back(Bucket()); }

void TimingWheel::OnConnection(const TcpConnectionPtr& conn) {
  EntryPtr entry(new Entry(conn));
  connection_buckets_.back().insert(entry);
  WeakEntryPtr weak_entry(entry);
  assert(conn->timing_context().empty());
  conn->SetTimingContext(weak_entry);
}

void TimingWheel::OnMessage(const TcpConnectionPtr& conn) {
  assert(!conn->timing_context().empty());
  WeakEntryPtr weak_entry(AnyCast<WeakEntryPtr>(conn->timing_context()));
  EntryPtr entry(weak_entry.lock());
  assert(entry);
  connection_buckets_.back().insert(entry);
}

}  // namespace net

}  // namespace mytinyhttpd