//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_TCP_SERVER_H
#define RPCFRAME_TCP_SERVER_H

#include <set>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_acceptor.h"
#include "net/tcp/tcp_buffer.h"
#include "net/tcp/tcp_client.h"
#include "net/eventloop.h"
#include "net/io_thread_pool.h"
#include "net/tcp/tcp_connection.h"
#include "net/coder/abstract_protocol.h"
#include "net/rpc/abstract_dispatcher.h"
#include "net/rpc/dispatcher/http_dispatcher.h"
#include "net/rpc/dispatcher/tinypb_dispatcher.h"

#define MAX_THREAD_POOL_SIZE 4
#define TIMER_EVENT_INTERVAL 5000

namespace rocket {
    class TCPServer{
    public:
        TCPServer(NetAddr::net_addr_sptr_t_ local_addr, NetAddr::net_addr_sptr_t_ register_center_addr,
                  ProtocolType protocol = ProtocolType::TinyPB_Protocol);

        ~TCPServer();

        void start();

        void registerService(TinyPBDispatcher::protobuf_service_sptr_t_ service);

        void registerToCenterAndStartLoop();

        void updateToCenter();

    private:
        void init();

        // 当有新客户端连接之后需要执行
        void onAccept();

        // 定时清理已经关闭了的客户端连接
        void clearClientTimerFunc();

    private:
        NetAddr::net_addr_sptr_t_ m_local_addr; // 本地监听地址
        TCPAcceptor::tcp_acceptor_sptr_t_ m_acceptor;
        EventLoop::event_loop_sptr_t_ m_main_event_loop{nullptr};
        std::unique_ptr<IOThreadPool> m_io_thread_pool{nullptr};

        // 应该都改成timer event info那样的shared ptr声明方式
        FDEvent::fd_event_sptr_t_ m_listen_fd_event;
        TimerEventInfo::time_event_info_sptr_t_ m_clear_client_timer_event;
        std::set<TCPConnection::tcp_connection_sptr_t_> m_client_connection;
        int m_client_counts{0};

        ProtocolType m_protocol_type;
        AbstractDispatcher::abstract_disp_sptr_t m_dispatcher{nullptr};
        AbstractCoder::abstract_coder_sptr_t_ m_coder{nullptr};
    private:
        // TCPClient::tcp_client_sptr_t_ m_client{nullptr}; // 主动向注册中心注册，或者更新到注册中心
        NetAddr::net_addr_sptr_t_ m_register_center_addr; // 本地监听地址
    };
}


#endif //RPCFRAME_TCP_SERVER_H
