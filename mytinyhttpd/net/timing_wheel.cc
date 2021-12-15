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
  if (conn->IsConnected()) {
    EntryPtr entry(
        new Entry(connection_buckets_.begin() - buckets_begin_, conn));
    connection_buckets_.back().insert(entry);
    WeakEntryPtr weak_entry(entry);
    assert(conn->timing_context().empty());
    conn->SetTimingContext(weak_entry);
  } else {
    assert(!conn->timing_context().empty());
    WeakEntryPtr weak_entry(AnyCast<WeakEntryPtr>(conn->timing_context()));
    LOG_DEBUG << "Entry use_count = " << weak_entry.use_count();
  }
}

void TimingWheel::OnMessage(const TcpConnectionPtr& conn, Timestamp receive_time) {
  assert(!conn->timing_context().empty());
  WeakEntryPtr weak_entry(AnyCast<WeakEntryPtr>(conn->timing_context()));
  EntryPtr entry(weak_entry.lock());
  assert(entry);
  if (entry->index_ != connection_buckets_.begin() - buckets_begin_) {
    entry->index_ = connection_buckets_.begin() - buckets_begin_;
    connection_buckets_.back().insert(entry);
  }
}

}  // namespace net

}  // namespace mytinyhttpd