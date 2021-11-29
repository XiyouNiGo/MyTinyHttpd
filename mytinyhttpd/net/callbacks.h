#ifndef MYTINYHTTPD_NET_CALLBACKS_H_
#define MYTINYHTTPD_NET_CALLBACKS_H_

#include <functional>
#include <memory>

#include "mytinyhttpd/utils/timestamp.h"

namespace mytinyhttpd {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace net {

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)>
    HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>
    MessageCallback;

void DefaultConnectionCallback(const TcpConnectionPtr& conn);
void DefaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer,
                            Timestamp receive_time);

}  // namespace net

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_NET_CALLBACKS_H_
