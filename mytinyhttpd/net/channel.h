#ifndef MYTINYHTTPD_NET_CHANNEL_H_
#define MYTINYHTTPD_NET_CHANNEL_H_

#include <functional>
#include <memory>

#include "mytinyhttpd/utils/noncopyable.h"
#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

namespace net {

class EventLoop;

class Channel : public noncopyable {
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;

  enum Status : int {
    kNew = -1,
    kAdded,    // useless for poll
    kDeleted,  // indicate that just been remove from epollfd (owing to
               // IsNoneEvent is true), but still exists in channel map, can be
               // added again, useless for poll
  };

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void HandleEvent(Timestamp receive_time);
  void SetReadCallback(ReadEventCallback cb) { read_callback_ = std::move(cb); }
  void SetWriteCallback(EventCallback cb) { write_callback_ = std::move(cb); }
  void SetCloseCallback(EventCallback cb) { close_callback_ = std::move(cb); }
  void SetErrorCallback(EventCallback cb) { error_callback_ = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void Tie(const std::shared_ptr<void>&);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void SetRevents(int revt) { revents_ = revt; }  // used by pollers
  int revents() const { return revents_; }
  bool IsNoneEvent() const { return events_ == kNoneEvent; }

  void EnableReading() {
    events_ |= kReadEvent;
    Update();
  }
  void DisableReading() {
    events_ &= ~kReadEvent;
    Update();
  }
  void EnableWriting() {
    events_ |= kWriteEvent;
    Update();
  }
  void DisableWriting() {
    events_ &= ~kWriteEvent;
    Update();
  }
  void DisableAll() {
    events_ = kNoneEvent;
    Update();
  }
  bool IsWriting() const { return events_ & kWriteEvent; }
  bool IsReading() const { return events_ & kReadEvent; }

  // for Poller:
  //  status for epoll, index for poll
  Status status() { return status_; }
  void SetStatus(Status status) { status_ = status; }

  std::string ReventsToString() const;
  std::string EventsToString() const;

  void DoNotLogHup() { is_log_hup_ = false; }

  EventLoop* loop() { return loop_; }
  void Remove();

 private:
  static std::string EventsToString(int fd, int ev);

  void Update();
  void HandleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  Status status_;  // used by Poller.
  bool is_log_hup_;

  std::weak_ptr<void> tie_;
  bool is_tied_;
  bool is_event_handling_;
  bool is_added_to_loop_;
  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
};

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_CHANNEL_H_
