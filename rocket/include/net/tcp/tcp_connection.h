//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_TCP_CONNECTION_H
#define RPCFRAME_TCP_CONNECTION_H

#include <memory>
#include <unordered_map>
#include <queue>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_buffer.h"
#include "net/io_thread.h"
#include "net/fd_event_pool.h"
#include "net/coder/abstract_protocol.h"
#include "net/coder/abstract_coder.h"
#include "net/rpc/abstract_dispatcher.h"

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

    class TCPConnection{
    public:
        using tcp_connection_sptr_t_ = std::shared_ptr<TCPConnection>;
    public:
        TCPConnection(EventLoop::event_loop_sptr_t_ event_loop, int client_fd, int buffer_size,
                      NetAddr::net_addr_sptr_t_ peer_addr,
                      NetAddr::net_addr_sptr_t_ local_addr,
                      AbstractCoder::abstract_coder_sptr_t_ coder,
                      AbstractDispatcher::abstract_disp_sptr_t dispatcher,
                      TCPConnectionType type = TCPConnectionByServer,
                      ProtocolType protocol = ProtocolType::TinyPB_Protocol);

        ~TCPConnection();

        // 读取数据，组装为rpc请求
        // execute，将rpc请求作为入参，执行业务逻辑得到rpc响应
        // write，将rpc响应返回给客户端
        void onRead();

        void execute();

        void onWrite();

        // 往connection中写入message，然后放入到m write dones中，因为emplace时候会复制，所以没必要调用时候再复制一遍
        // 直接标记为const和引用传递就可以
        void pushSendMessage(const AbstractProtocol::abstract_pro_sptr_t_ &message,
                             const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done);

        void pushReadMessage(const std::string &msg_id,
                             const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done);

        void setState(TCPState new_state);

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
        EventLoop::event_loop_sptr_t_ m_event_loop;
        NetAddr::net_addr_sptr_t_ m_local_addr;
        NetAddr::net_addr_sptr_t_ m_peer_addr;
        TCPBuffer::tcp_buffer_sptr_t_ m_in_buffer; // 接收缓冲区
        TCPBuffer::tcp_buffer_sptr_t_ m_out_buffer; // 发送缓冲区
        FDEvent::fd_event_sptr_t_ m_fd_event{nullptr};
        TCPState m_state;
        int m_client_fd{0};
        TCPConnectionType m_connection_type{TCPConnectionByServer};

        // coder，使用多态
        AbstractCoder::abstract_coder_sptr_t_ m_coder{nullptr};
        // dispatcher, 使用多态
        AbstractDispatcher::abstract_disp_sptr_t m_dispatcher{nullptr};

        // 按顺序进行写入，需要存上智能指针，因为回调传入的也是个abstract_pro_sptr_t_的智能指针
        std::vector<std::pair<AbstractProtocol::abstract_pro_sptr_t_,
                std::function<void(AbstractProtocol::abstract_pro_sptr_t_)>>> m_write_dones;

        // key为msg id
        std::unordered_map<std::string,
                std::function<void(AbstractProtocol::abstract_pro_sptr_t_)>> m_read_dones;

        ProtocolType m_protocol_type;
    };

}


#endif //RPCFRAME_TCP_CONNECTION_H
