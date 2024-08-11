//
// Created by baitianyu on 8/11/24.
//

#ifndef RPCFRAME_REGISTER_CENTER_H
#define RPCFRAME_REGISTER_CENTER_H

#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_acceptor.h"
#include "net/tcp/tcp_connection.h"
#include "net/tcp/tcp_client.h"
#include "net/eventloop.h"
#include "net/io_thread_pool.h"
#include "net/rpc/dispatcher/register_dispatcher.h"

#define MAX_THREAD_POOL_SIZE 4
#define TIMER_EVENT_INTERVAL 5000
#define UPDATE_SERVER_TIMER_EVENT_INTERVAL 5000

namespace rocket {

    class RegisterCenter : public std::enable_shared_from_this<RegisterCenter> {
    public:
        explicit RegisterCenter(NetAddr::net_addr_sptr_t_ local_addr,
                                ProtocolType protocol = ProtocolType::TinyPB_Protocol);

        ~RegisterCenter();

        void start();

        TCPClient *GetClient();

    private:

        void init();

        void onAccept();

        void clearClientTimerFunc();

        void updateServerMethod();

    private:
        // 作为服务器端接受服务端的注册，以及客户端的访问
        NetAddr::net_addr_sptr_t_ m_local_addr;
        TCPAcceptor::tcp_acceptor_sptr_t_ m_acceptor;
        EventLoop::event_loop_sptr_t_ m_main_event_loop{nullptr};
        std::unique_ptr<IOThreadPool> m_io_thread_pool{nullptr};
        FDEvent::fd_event_sptr_t_ m_listen_fd_event;
        TimerEventInfo::time_event_info_sptr_t_ m_clear_client_timer_event;
        ProtocolType m_protocol_type;
        std::set<TCPConnection::tcp_connection_sptr_t_> m_client_connection;
        int m_client_counts{0};
        // dispatcher1: 接受客户端的请求，并在dispatcher中查找该method对应的server(后期可加负载均衡)，并返回对应server的ip和port
        // dispatcher2: 接受服务端的请求，并在dispatcher中注册server和其所有的method，返回ok
        std::shared_ptr<RegisterDispatcher> m_dispatcher{nullptr};
        AbstractCoder::abstract_coder_sptr_t_ m_coder{nullptr};
    private:
        // 作为客户端向服务端请求更新数据，使用http协议后，主动connect对方服务器
        TCPClient::tcp_client_sptr_t_ m_client{nullptr};
        TimerEventInfo::time_event_info_sptr_t_ m_update_server_timer_event;
    };

}

#endif //RPCFRAME_REGISTER_CENTER_H
