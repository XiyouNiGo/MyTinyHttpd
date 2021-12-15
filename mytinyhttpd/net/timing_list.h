#ifndef MYTINYHTTPD_NET_TIMING_LIST_H_
#define MYTINYHTTPD_NET_TIMING_LIST_H_

#include <list>
#include <memory>

#include "mytinyhttpd/net/callbacks.h"
#include "mytinyhttpd/net/timing.h"
#include "mytinyhttpd/utils/copyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class TimingList : public Timing {
 public:
  TimingList(int idle_seconds = 8) : idle_seconds_(idle_seconds) {}
  ~TimingList() override = default;

  void OnTimer() override;

  void OnConnection(const TcpConnectionPtr& conn) override;

  void OnMessage(const TcpConnectionPtr& conn, Timestamp receive_time) override;

 private:
  typedef std::list<WeakTcpConnectionPtr> WeakConnectionList;

  struct Node : public copyable {
    Timestamp last_receive_time;
    WeakConnectionList::iterator position;
  };

  int idle_seconds_;
  WeakConnectionList connection_list_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_TIMING_LIST_H_