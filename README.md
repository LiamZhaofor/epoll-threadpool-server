
# epoll-threadpool-server

一个基于 C++17 和 Linux Socket 的服务器项目。  
当前版本已完成 **阻塞式 TCP Echo Server** 的基础框架，后续将逐步演进为 **non-blocking + epoll + thread pool** 的高并发服务器。

## 项目目标

这个项目的目标不是一开始就直接写成完整的高并发服务器，而是按照工程化路线逐步演进：

1. 先实现阻塞式 TCP Server，熟悉 Socket 编程基本流程
2. 再升级为非阻塞 I/O
3. 引入 epoll 实现 I/O 多路复用
4. 加入线程池，将 I/O 与业务处理解耦
5. 最终形成一个可扩展的 Linux 高并发服务器原型

## 当前进度

目前已完成：

- 基础 TCP 服务端封装
- 监听 socket 创建、bind、listen
- 阻塞式 accept / recv / send
- Echo 回显逻辑
- 基于 CMake 的基础构建流程

当前程序入口通过 `RunBlockingEcho()` 启动服务，因此现阶段项目属于**第一阶段：阻塞式服务器原型**。  
后续会继续加入 epoll、线程池、连接管理、日志与压力测试等模块。

## 技术栈

- C++17
- Linux Socket API
- CMake
- TCP/IP
- 多线程（计划中）
- epoll（计划中）

## 项目结构

```text
epoll-threadpool-server
├── include
│   └── tcp_server.h
├── src
│   ├── main.cpp
│   └── tcp_server.cpp
├── build
├── CMakeLists.txt
└── README.md
````

### 目录说明

* `include/`：头文件，声明服务器接口
* `src/`：源文件，包含程序入口和服务器实现
* `build/`：构建输出目录
* `CMakeLists.txt`：项目构建配置

## 核心流程

当前版本的服务器运行流程如下：

```text
socket
  ↓
bind
  ↓
listen
  ↓
accept
  ↓
recv
  ↓
send
```

程序启动后，服务端监听指定端口，接收客户端连接，并将客户端发送的数据原样返回（Echo）。

## 构建与运行

### 1. 克隆项目

```bash
git clone https://github.com/LiamZhaofor/epoll-threadpool-server.git
cd epoll-threadpool-server
```

### 2. 编译

```bash
mkdir -p build
cd build
cmake ..
make
```

### 3. 运行

```bash
./server
```

默认监听地址与端口由当前代码写死为：

* IP: `0.0.0.0`
* Port: `8080`

## 测试方法

可以使用 `telnet` 或 `nc` 连接服务器进行简单测试。

### 使用 nc

```bash
nc 127.0.0.1 8080
```

连接后输入任意内容，服务器会将收到的数据原样返回。

## 当前版本特点

### 已实现

* 基础 TCP 服务端封装
* 阻塞式通信模型
* Echo 回显
* 错误处理与基本日志输出

### 当前限制

* 仍为阻塞式模型
* 暂不支持高并发连接优化
* 暂未引入 epoll
* 暂未加入线程池
* 暂未实现连接对象管理与超时清理

## 后续开发计划

接下来准备按以下路线推进：

### 第一阶段

* [x] 阻塞式 TCP Echo Server
* [x] CMake 工程化构建

### 第二阶段

* [ ] 将 listenfd / connfd 设置为非阻塞
* [ ] 引入 epoll 实现事件驱动模型
* [ ] 支持多个客户端并发连接

### 第三阶段

* [ ] 实现线程池
* [ ] 将业务处理从主线程中拆分
* [ ] 优化服务器整体结构

### 第四阶段

* [ ] 封装 Connection 类
* [ ] 增加读写缓冲区
* [ ] 处理粘包 / 半包问题
* [ ] 增加日志模块
* [ ] 增加超时连接管理
* [ ] 添加压力测试

## 项目意义

这个项目主要用于系统学习和实践以下内容：

* Linux 网络编程
* TCP Socket 通信模型
* 阻塞与非阻塞 I/O
* I/O 多路复用
* 多线程与线程池
* 高并发服务器基础架构设计

它既是一个代码项目，也是一个从基础网络编程逐步走向高并发服务器设计的实践过程。

## 说明

当前仓库仍处于早期开发阶段。
仓库名称为 `epoll-threadpool-server`，但目前公开代码主要完成的是**阻塞式 Echo Server 基础框架**。后续会继续迭代 epoll 与线程池相关模块，使项目名称与实现能力逐步对应。

## License

This project is for learning and practice.


