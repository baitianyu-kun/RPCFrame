//
// Created by baitianyu on 2/12/25.
//

#ifndef RPCFRAME_RPC_PUBLISH_LISTENER_H
#define RPCFRAME_RPC_PUBLISH_LISTENER_H

#include <memory>
#include <functional>
#include "net/tcp/tcp_acceptor.h"
#include "event/eventloop.h"
#include "net/protocol/http/http_define.h"
#include "net/tcp/tcp_connection.h"

namespace mrpc {

    class PublishListenerRunner {
    public:
        using ptr = std::shared_ptr<PublishListenerRunner>;

        using callback = std::function<void(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                            HTTPSession::ptr session)>;

        explicit PublishListenerRunner(NetAddr::ptr local_addr);

        ~PublishListenerRunner();

        void addServlet(const std::string &uri, Servlet::ptr slt);

        void addServlet(const std::string &uri, CallBacksServlet::callback cb);

        void start();

    private:
        void init();

        void onAccept();

    private:
        NetAddr::ptr m_local_addr;
        TCPAcceptor::ptr m_acceptor;
        EventLoop::ptr m_main_event_loop;
        FDEvent::ptr m_listen_fd_event;
        RPCDispatcher::ptr m_dispatcher;
        TCPConnection::ptr m_connection; // onAccept之后必须存一下，否则局部变量connection被销毁了
    };

    class PublishListener {
    public:
        using ptr = std::shared_ptr<PublishListener>;

        using callback = std::function<void(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                            HTTPSession::ptr session)>;

        PublishListener(NetAddr::ptr local_addr, callback call_back);

        ~PublishListener();

    private:
        static void *runner(void *arg);

        PublishListenerRunner::ptr getPublishListenerRunner() const { return m_runner; };

    private:
        pid_t m_thread_id{-1};
        pthread_t m_thread{0};
        NetAddr::ptr m_local_addr;
        callback m_callback; // 接收到注册中心的推送后需要执行的方法，由channel进行绑定
        PublishListenerRunner::ptr m_runner;
    };
}

#endif //RPCFRAME_RPC_PUBLISH_LISTENER_H
