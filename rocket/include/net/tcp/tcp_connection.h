//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_TCP_CONNECTION_H
#define RPCFRAME_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_buffer.h"
#include "net/io_thread.h"
#include "net/fd_event_pool.h"

namespace rocket {
    enum TCPState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4
    };

    enum TCPConnectionType {
        TCPConnectionByServer = 1,  // 作为服务端使用，代表跟对端客户端的连接
        TCPConnectionByClient = 2,  // 作为客户端使用，代表跟对端服务端的连接
    };

    class TCPConnection {
    public:
        using tcp_connection_sptr_t_ = std::shared_ptr<TCPConnection>;
    public:
        TCPConnection(const std::unique_ptr<EventLoop> &event_loop, int client_fd, int buffer_size,
                      NetAddr::net_addr_sptr_t_ peer_addr,
                      NetAddr::net_addr_sptr_t_ local_addr, TCPConnectionType type = TCPConnectionByServer);

        ~TCPConnection();

        // 读取数据，组装为rpc请求
        // execute，将rpc请求作为入参，执行业务逻辑得到rpc响应
        // write，将rpc响应返回给客户端
        void onRead();

        void execute();

        void onWrite();

        void setState(const TCPState new_state);

        TCPState getState();

        void clear();

        int getFD();

        // 服务器主动关闭连接
        void shutdown();

        void setConnectionType(TCPConnectionType type);

        // 监听可写事件
        void listenWrite();

        // 监听可读事件
        void listenRead();

        // 监听地址
        NetAddr::net_addr_sptr_t_ getLocalAddr();

        // 连接的客户端地址
        NetAddr::net_addr_sptr_t_ getPeerAddr();

    private:
        // 这里使用的是io thread里面创建的event loop，所以创建一个指针共同管理，使用unique的get方法
        // 假如我们只需要在函数中，用这个对象处理一些事情，但不打算涉及其生命周期的管理，也不打算通过函数传参延长 shared_ptr 的生命周期。
        // 对于这种情况，可以使用 raw pointer 或者 const shared_ptr&。
        // 这里的connection只使用eventloop，而不是销毁，销毁是io thread里管理的，所以这里可以用裸指针，或者是传递unique ptr的引用
        const std::unique_ptr<EventLoop> &m_event_loop;
        NetAddr::net_addr_sptr_t_ m_local_addr;
        NetAddr::net_addr_sptr_t_ m_peer_addr;
        TCPBuffer::tcp_buffer_sptr_t_ m_in_buffer; // 接收缓冲区
        TCPBuffer::tcp_buffer_sptr_t_ m_out_buffer; // 发送缓冲区
        FDEvent::fd_event_sptr_t_ m_fd_event{nullptr};
        TCPState m_state;
        int m_client_fd{0};
        TCPConnectionType m_connection_type{TCPConnectionByServer};
    };

}


#endif //RPCFRAME_TCP_CONNECTION_H
