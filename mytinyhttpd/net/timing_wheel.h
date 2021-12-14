#ifndef MYTINYHTTPD_NET_TIMING_WHEEL_H_
#define MYTINYHTTPD_NET_TIMING_WHEEL_H_

#include <boost/circular_buffer.hpp>
#include <boost/circular_buffer/base.hpp>
#include <memory>
#include <unordered_set>

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class TimingWheel : public noncopyable {
 public:
  TimingWheel(int idle_seconds = 8)
      : connection_buckets_(
            idle_seconds,
            Bucket()),  // without this, program will dump in first second
        buckets_begin_(connection_buckets_.begin()) {}
  ~TimingWheel() = default;

  void OnTimer();

  void OnConnection(const TcpConnectionPtr& conn);

  void OnMessage(const TcpConnectionPtr& conn);

 private:
  struct Entry : public copyable {
    explicit Entry(long index, const WeakTcpConnectionPtr& weak_conn)
        : index_(index), weak_conn_(weak_conn) {}
    ~Entry();

    long index_;
    WeakTcpConnectionPtr weak_conn_;
  };

  typedef std::shared_ptr<Entry> EntryPtr;
  typedef std::weak_ptr<Entry> WeakEntryPtr;
  typedef std::unordered_set<EntryPtr> Bucket;
  typedef boost::circular_buffer<Bucket> WeakConnectionList;

  WeakConnectionList connection_buckets_;
  WeakConnectionList::iterator buckets_begin_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMING_WHEEL_H_