# MyTinyHttpd
## 项目简述
+ MyTinyHttpd 是采用 C++11 编写，基于 Linux 平台的高性能高并发 Web 服务器，支持 1.1 版本。
+ 项目采用 one-loop-per-thread 模型开发，使用 Round-robin 算法对主 Reactor 的连接进行分配；采用eventfd 和 epoll 进行线程切换，保证并发安全性；采用 timerfd 和 TimerQueue 进行定时；实现 Buffer 类方便这种异步回调的网络机制使用；采用 RAII 机制管理资源生命周期；采用多缓冲区技术实现多线程异步日志；采用空闲描述符来限制并发连接数；支持 Json 文件配置服务器；采用 CMake 集成 Gtest、Google benchmark 来进行单元测试和性能测试。
## 技术要点
### one-loop-per-thread + epoll LT
  + Main Reactor负责accept连接，然后把连接挂在某个Sub Reactor中（采用round-robin的方式来选择 Sub Reactor），这样该连接的所有操作都在那个Sub Reactor所处的线程中完成。多个连接可能被分派到多个线程中，以充分利用CPU。
  + 采用固定大小的Reactor pool，池子的大小通常根据CPU数目确定，也就是说线程数是固定，这样程序的总体处理能力不会随着连接数增加而下降。
  + 一个连接完全由一个线程管理，请求的顺序性有保证，突发请求也不会占满全部CPU。
  + 小规模计算可以在当前IO线程完成并发回结果，从而降低响应的延迟。
  + MyTinyHttpd作为一个Web服务器，主要用于处理I/O密集型任务，在这种基于事件的非阻塞网络编程模型下，在某个I/O线程停留的时间并不会很长，能够保证尽快将执行流交给别的连接。
  + 选择epoll LT模式第一是为了与poll兼容（Poller同时支持poll和epoll两种多路复用方式），第二是可以节省一次系统调用，读写时不用等到每次都出现EAGAIN。
### 基于对象的Reactor模型
+ Reactor模型的主要成员为：EventLoop（事件循环的抽象）、Poller（I/O复用的抽象）、Channel（事件分发的抽象）、Accept（被动连接的抽象）、Connector（主动连接的抽象）、TcpConnection（Tcp连接的抽象）。
+ EventLoop与Poller的关系是组合，Poller的生存期由EventLoop来控制。EventLoop调用Poller来进行轮询。EventLoop和Channel的关系是聚合，一个EventLoop包含多个Channel，但不负责控制Channel的生命周期，Channel的生命周期由TcpConnection、Acceptor、Connector来控制。Channel更新事件时会由EventLoop调用Poller的updateChannel函数。
+ Channel和Socket是TcpConnection、Acceptor、Connector的成员，它们的关系为组合。而Channel虽然用到Socket中的文件描述符，它们是一一对应的关联关系。上层的类向Channel注册HandleXXX函数，由EventLoop调用Channel的HandleEvent，对发生的事件进行响应。
+ Accept和Connector分别与上层的TcpServer和TcpClient是组合关系，Channel调用Accept的回调函数后，Accept会调用TcpServer的回调函数来建立新连接。
+ TcpServer与TcpConnection的关系是聚合，TcpConnection的生命周期采用shared_ptr控制，引用计数降到0后自动析构。
### 转移到I/O线程执行任务
+ 该功能具体由EventLoop::RunInloop实现，如果用户在当前I/O线程调用该函数，回调会同步进行；如果在其他线程调用，任务会被加入队列，I/O线程会被唤醒来执行任务。同时提供了QueueInLoop，直接加入任务队列等待调用，这样在一些场景下能有效避免爆栈（例如在WriteCompleteCallback函数中Send很小的数据包）。
+ EventLoop使用了eventfd来统一事件源，防止将任务暴露在signal handler中而引起死锁等问题。
+ QueueInLoop除了转移任务到其他线程，还可以防止非法访问，典型的用法是在Connector的RemoveAndResetChannel函数中，该函数可能由Channel的回调函数调用，因此不能析构自身，需要调用QueueInLoop随后重置Channel。
### RAII封装
+ RAII也称为“资源获取就是初始化”，是C++语言的一种管理资源、避免泄漏的惯用法。RAII 的做法是使用一个对象，在其构造时获取资源，在对象生命期控制对资源的访问使之始终保持有效，最后在对象析构的时候释放资源。
+ 使用RAII封装对象从而避免遗忘释放资源，或在非常规情况下无法释放资源的问题，毕竟C++标准保证了对象的析构。
+ 由于RAII对象为栈上对象，只需查看调用栈就能分析该资源的分配情况。
+ 因此，MyTinyHttpd中的常见同步原语，以及Socket等对象都采用该手法封装，Socket封装能防止串话（避免使用带锁的全局表）和多次close之类的错误使用问题。
### 多缓冲异步日志
+ 该模块将日志写入分为前台线程和后台线程，后台线程只有一个，负责写入日志，前台线程负责生成日志但只写入缓冲区等待后台线程交换（swap），而不写入实际文件，开销低且不会阻塞。后台进程只操作一个带缓冲的文件，用fwrite_unlock来写，效率较高，且减少了I/O次数，不必每次都写文件，每隔一天或者超过一定大小则会回滚文件，生成新的日志文件。
+ 为了保证日志消息的及时性，即便current_buffer未满，后台线程也会每三秒执行一次（实际上调用pthread_cond_timewait来定时）上述交换写入操作。
### 应用层Buffer
+ 应用程序需要发送的数据大于操作系统的TCP缓冲区时，应该把多余数据放到这个TCP链接的应用层发送缓冲区中，因此需要一个高性能的应用层缓冲。
+ Buffer类并不是线程安全的，因为一个Buffer应该只属于一个TcpConnection，而收发数据时，TcpConnection会借助RunInLoop转到I/O线程来操作Buffer。
+ Buffer使用readv结合栈上空间来减少缓冲区所带来的开销，也能够帮助我们减少系统调用次数。
+ Buffer类使用两个int类型的index（使用指针会有迭代器失效的问题）将vector的内容分为prependable、readable、writable，其中prependable为预分配的一小块空间，用于在缓冲区前添加某些字长字段，避免重新分配空间。
### 限制并发连接数
+ 当用accept系统调用获得新连接的socket文件描述符时，如果文件描述符已经耗尽，则会返回EMFILE，然而无法创建socket文件描述符意味着我们无法close它，采用ET模式时，这会导致busy loop。
+ 具体的做法是使用一个空闲的文件描述符，先关闭它，调用accept获得新连接的文件描述符，随后关闭它，最后重新打开一个空闲文件以作备用。
### 定时器
+ 在x86-64平台上，gettimeofday是在用户态实现，没有上下文切换和陷入内核的开销，且有足够的精度（struct timeval精度为微秒），因此我们选择它来获取当前时间。
+ timerfd_create系统调用把时间变成文件描述符，方便融入EventLoop中，用统一的方式来处理I/O事件和超时事件，因为我们选择它来定时。
+ 定时器采用的数据结构为二叉搜索树（std::set），配合std::pair<Timstamp, Timer*>来处理两个Timer到期时间相同的问题。
+ 由于Linux是非实时多任务操作系统，定时器只做到基本精确。
### 踢掉空闲连接
+ HttpServer支持用Timing Wheel或者Timing List踢掉空闲连接，具体选择由环境变量确定。
+ Timing Wheel本质上是一个circlar_buffer模拟的循环队列，每一个格子是一个hash set，往队尾添加空桶会自动弹出队首的Bucket，并析构。桶里放的不是TcpConnection，而是特制的Entry类（也用shared_ptr管理），包含TcpConnection的虚指针，析构时会先尝试lock，如果提升成功说明该连接仍然存在，调用shutdown关闭，否则什么也不做。当收到消息时，从TcpConnection的context中取出Entry的弱引用，而不是新的Entry，否则无法延长Entry的生命周期，导致TcpConnection的提前关闭。
+ Timing List是一个时间链表，按接受时间先后排序，每次收到消息就移动到链表尾部，再用一个定时器定期从链表前查找并踢掉超时的连接。
### TcpConnection生命周期管理
+ TcpConnection类生命周期模糊，何时析构成了一个大问题，因此采用shared_ptr来管理，当引用计数为0时，TcpConnection对象析构，连接被关闭。
+ TcpConnection与上层的TcpServer或是TcpClient是聚合关系而非组合，当连接被客户端关闭时，引用计数为0，TcpConnection自动析构，具体过程如下：
	- 新连接创建后，存入ConnectionMap，引用计数为1。
	- 当连接关闭时，EventLoop会调用Channel的HandleEvent函数执行TcpConnection提前注册到Channel的HandleRead函数。
	- 在HandleRead中调用HandleClose，HandleClose中使用了shared_from_this()，引用计数为2。
	- 在HandleClose中调用RemoveConnection，从ConnectionMap中erase对应的连接，引用计数为1。
	- 使用bind来调用ConnectDestroyed时，引用计数为2。
	- 当HandleClose调用结束，引用计数减1变为1。
	- 最后ConnectDestroyed调用结束，引用计数为0，对象析构。
## 相关项目
[http://github.com/chenshuo/muduo](http://github.com/chenshuo/muduo)