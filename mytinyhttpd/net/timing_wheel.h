#ifndef MYTINYHTTPD_NET_TIMING_WHEEL_H_
#define MYTINYHTTPD_NET_TIMING_WHEEL_H_

#include <boost/circular_buffer.hpp>
#include <memory>
#include <unordered_set>

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class TimingWheel : public noncopyable {
 public:
  TimingWheel(int idle_seconds = 8) : connection_buckets_(idle_seconds) {}
  ~TimingWheel() = default;

  void OnTimer();

  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn);

 private:
  struct Entry : public copyable {
    explicit Entry(const WeakTcpConnectionPtr& weak_conn)
        : weak_conn_(weak_conn) {}
    ~Entry();

    WeakTcpConnectionPtr weak_conn_;
  };

  typedef std::shared_ptr<Entry> EntryPtr;
  typedef std::weak_ptr<Entry> WeakEntryPtr;
  typedef std::unordered_set<EntryPtr> Bucket;
  typedef boost::circular_buffer<Bucket> WeakConnectionList;

  WeakConnectionList connection_buckets_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMING_WHEEL_H_