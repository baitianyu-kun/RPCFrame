//
// Created by baitianyu on 7/22/24.
//

#ifndef RPCFRAME_TCP_CLIENT_H
#define RPCFRAME_TCP_CLIENT_H

#include <memory>
#include "net/tcp/net_addr.h"
#include "net/eventloop.h"
#include "net/tcp/tcp_connection.h"
#include "net/timer_fd_event.h"
#include "net/coder/abstract_protocol.h"

namespace rocket {

    // TCPServer是以服务器的身份提供服务
    // TCPClient是以客户端的身份去访问其他人的服务
    // 所以这里主要实现的就是主动连接其他人
    class TCPClient {
    public:
        using tcp_client_sptr_t_ = std::shared_ptr<TCPClient>;

        explicit TCPClient(NetAddr::net_addr_sptr_t_ peer_addr);

        ~TCPClient();

        // 异步的进行connect，如果connect完成则done会被执行
        void connect(std::function<void()> done);

        // 异步的发送message，发送message成功的话，会调用done函数，done函数的入参就是message对象
        // 将message对象写入到tcp connection中的out buffer中，同时done函数也要写入进去，发送完后就可以找到对应的回调函数去执行
        // 然后启动connection可写事件就可以
        void writeMessage(const AbstractProtocol::abstract_pro_sptr_t_& message,
                          const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)>& done);

        // 异步的读取message，如果读取成果，会调用done函数，函数的入参就是message对象
        void readMessage(const std::string &msg_id,
                         const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)>& done);

        void stop();

        NetAddr::net_addr_sptr_t_ getPeerAddr();

        NetAddr::net_addr_sptr_t_ getLocalAddr();

    private:
        NetAddr::net_addr_sptr_t_ m_peer_addr;
        NetAddr::net_addr_sptr_t_ m_local_addr;
        std::unique_ptr<EventLoop> m_event_loop;
        int m_client_fd{-1};
        FDEvent::fd_event_sptr_t_ m_fd_event;
        TCPConnection::tcp_connection_sptr_t_ m_connection;
    };

}


#endif //RPCFRAME_TCP_CLIENT_H
