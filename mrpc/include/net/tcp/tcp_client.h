//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_TCP_CLIENT_H
#define RPCFRAME_TCP_CLIENT_H

#include "net/tcp/net_addr.h"
#include "event/eventloop.h"
#include "net/tcp/tcp_connection.h"

namespace mrpc {

    class TCPClient {
    public:
        using ptr = std::shared_ptr<TCPClient>;

        // 默认获取当前线程的eventloop
        // 也可以指定获取哪个线程的eventloop
        TCPClient(NetAddr::ptr peer_addr, EventLoop::ptr specific_eventloop = EventLoop::GetCurrentEventLoop());

        ~TCPClient();

        void connect(std::function<void()> done);

        void sendRequest(const HTTPRequest::ptr &request, const std::function<void(HTTPRequest::ptr)> &done);

        void recvResponse(const std::string &msg_id, const std::function<void(HTTPResponse::ptr)> &done);

        NetAddr::ptr getPeerAddr();

        NetAddr::ptr getLocalAddr();

        void setPeerAddr(NetAddr::ptr new_peer_addr);

        int getConnectErrorCode() const;

        std::string getConnectErrorInfo();

        void initLocalAddr();

        int getClientFD() const;

        EventLoop::ptr getEventLoop();

        bool setSocketOption(int level, int option, void* result, size_t len);

        template<class T>
        bool setSocketOption(int level, int option, T* result)  {
            return setSocketOption(level, option, result, sizeof(T));
        }

    private:
        NetAddr::ptr m_peer_addr;
        NetAddr::ptr m_local_addr;
        EventLoop::ptr m_event_loop;
        int m_client_fd{-1};
        FDEvent::ptr m_fd_event;
        TCPConnection::ptr m_connection;
        int m_connect_err_code{0};
        std::string m_connect_err_info;

    };

}

#endif //RPCFRAME_TCP_CLIENT_H
