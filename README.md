# epoll-threadpool-server

一个基于 C++17 和 Linux Socket 的网络服务器练手项目，按照“阻塞式 TCP -> 非阻塞 + epoll -> 线程池”的路线逐步演进。

当前版本已经完成：

- 非阻塞监听 socket 与连接 socket
- 基于 `epoll` 的事件驱动主循环
- 多客户端并发接入
- `Connection` 连接对象管理
- 基于 `\n` 分隔符的基础消息切分
- Echo 回显与 `quit` 主动退出

## 项目目标

这个项目的重点不是一次性写成工业级高并发服务器，而是通过逐步演进，把下面这些核心概念真正跑通并讲清楚：

1. Linux Socket 基础流程
2. 非阻塞 I/O
3. `epoll` 事件驱动模型
4. 连接对象管理
5. 线程池与 I/O / 业务解耦

## 当前进度

当前代码已经从最早的阻塞式 Echo Server 演进到一个可运行的 `epoll` 多连接 Echo Server，核心流程如下：

```text
socket
  ->
bind
  ->
listen
  ->
epoll_create1
  ->
epoll_ctl(listen_fd)
  ->
epoll_wait
  ->
accept / recv
  ->
Connection buffer
  ->
echo response
```

### 当前实现能力

- 主线程通过 `epoll_wait` 统一等待事件
- `listen_fd` 负责接收新连接
- 客户端连接通过 `Connection` 对象保存状态
- 每个连接拥有独立的读写缓冲区
- 收到的数据先进入 `read_buffer`
- 以 `\n` 作为消息边界，逐条切分并处理

### 当前仍未完成

- 线程池
- 写缓冲驱动的异步发送
- 更完善的日志系统
- 超时连接清理
- 压力测试

## 技术栈

- C++17
- Linux Socket API
- epoll
- CMake
- TCP/IP

## 项目结构

```text
epoll-threadpool-server
├── include
│   ├── connection.h
│   └── tcp_server.h
├── src
│   ├── connection.cpp
│   ├── main.cpp
│   └── tcp_server.cpp
├── test
├── CMakeLists.txt
├── rebuild.sh
├── run.sh
└── README.md
```

### 目录说明

- `include/`：头文件，声明服务器与连接对象接口
- `src/`：源文件，包含主逻辑实现
- `test/`：测试代码目录
- `run.sh`：增量构建并运行
- `rebuild.sh`：清理后重新构建并运行

## 核心设计

### 1. `TcpServer`

负责：

- 创建监听 socket
- 初始化 `epoll`
- 事件分发
- 新连接接入
- 连接关闭清理

### 2. `Connection`

负责保存单个连接的状态：

- `fd`
- `read_buffer`
- `write_buffer`

引入 `Connection` 后，服务器开始从“按 fd 处理”过渡到“按连接对象处理”，为后续解决粘包半包和引入线程池打下基础。

### 3. 基础协议处理

当前版本使用 `\n` 作为简单应用层分隔符：

- 一次 `recv` 可能收到半条消息
- 也可能收到多条消息
- 数据先写入 `Connection::read_buffer`
- 再循环查找 `\n`，逐条切分完整消息

这使项目具备了基础的粘包 / 半包处理能力。

## 构建与运行

### 1. 编译并运行

```bash
./run.sh
```

### 2. 清理后重新构建并运行

```bash
./rebuild.sh
```

默认监听：

- IP: `0.0.0.0`
- Port: `8080`

## 测试方法

使用 `nc` 连接：

```bash
nc 127.0.0.1 8080
```

输入：

```text
hello
world
quit
```

可以验证：

- 基础连接建立
- 多条消息逐条回显
- `quit` 主动退出

如果同时开启多个 `nc` 客户端，还可以验证多连接接入是否正常。

## 当前阶段总结

可以把当前版本定义为：

**一个基于 C++17、非阻塞 socket、epoll 和 Connection 管理的多客户端 Echo Server 原型。**

它已经具备：

- 事件驱动 I/O 模型
- 多连接接入
- 基础连接状态管理
- 简单的消息边界处理

下一步将进入线程池阶段，把业务处理从主事件循环中拆分出去。

## License

This project is for learning and practice.
