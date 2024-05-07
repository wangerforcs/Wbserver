## 目录结构

```
.
├── bin 编译产生的.o文件
├── include 头文件
├── src 源文件
├── log 日志文件
├── resources 静态资源
└── webbench-1.5 压力测试工具
```

## 功能
* 利用io复用epoll与线程池实现多线程的Reactor高并发模型；
* 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；
* 基于小根堆实现的定时器，关闭超时的非活动连接；
* 利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态；
* 利用RAII机制实现了数据库连接池，减少数据库连接建立与关闭的开销，实现了用户注册登录功能。


## 部件实现

### buffer
使用vector实现的非循环缓冲区

### Log
#### blockqueuq
阻塞队列，对STL的deque的各种操作实现多线程版本
#### log
记录运行期间的各种日志，分为同步和异步两种方式  
将要输出的各种信息直接放入buffer中  
同步模式直接将buffer中的信息输出到文件中，异步模式放入blockqueue中，由另一个线程输出到文件中。

代码如何让主线程和异步写线程共享一个对象：  
使用了Instance函数，返回一个静态指向对象的指针，任何时候都使用的同一个对象。

### timer
自己实现的定时器堆，包括添加，调整，立即执行，清除超时计时器等功能。

### pool
#### threadpool
主线程向队列添加任务，工作线程从队列取出任务执行。
#### sqlconnpool
提供了从本地连接队列获取连接和释放连接的借口。  
使用RAII机制，将数据库连接封装成类，使用时创建对象，析构时自动释放连接。

### http
#### request
解析请求行，请求头，请求体，并对post请求进行解析，以实现用户注册和登录。
#### response
生成响应报文，包括响应行，响应头，响应体。
#### httpconn
包含了request和response，以及对应存储数据的读写缓冲。使用iovec读写buffer和文件描述符。

### server
#### epoll
对内核事件表的操作进行封装，包括添加，修改，删除文件描述符



一个疑惑
看到的基本都是队列+回调函数实现任务队列，解析http请求的时候，每次函数都是从最开始的状态开始，如果客户端的一次完整的http请求在服务器端一次read就能读完，那么这样的实现是没有问题的，但如果可能的网络波动导致一次read不能完成request的全部读取，就会出错。
因此我进行了修改，线程会从上一个线程解析之后的状态开始解析，这样就不会出现上述问题。
但这样如何确定一个完整的http请求到哪里结束呢？好像以表单形式提交的POST请求，body部分也并没有结束符，但首部部分似乎有Content-Length字段。因此我主要对HttpConn::process和HttpRequest::parse进行了修改，解析parse的返回值代表是否完成整个请求的解析。

## 新建数据库
需要先配置好对应的数据库
```bash
create database yourdb;

USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

INSERT INTO user(username, password) VALUES('name', 'password');
```


## 压力测试
```bash
./webbench-1.5/webbench -c 并发数 -t 测试时间 http://ip:port/
```
* QPS 10000+


## 参考
Linux高性能服务器编程，游双著.


GET /index.html HTTP/1.1
Connection: keep-alive

