//
// Created by baitianyu on 2/12/25.
//

#ifndef RPCFRAME_RPC_PUBLISH_LISTENER_H
#define RPCFRAME_RPC_PUBLISH_LISTENER_H

#include "net/tcp/tcp_server.h"
#include "net/tcp/tcp_client.h"

namespace mrpc {

    class PublishListener : public TCPServer {
    public:
        using ptr = std::shared_ptr<PublishListener>;

        using callback = std::function<void(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                            HTTPSession::ptr session)>;

        PublishListener(NetAddr::ptr local_addr, callback call_back,
                        EventLoop::ptr specific_eventloop = EventLoop::GetCurrentEventLoop());

        ~PublishListener();

    private:
        NetAddr::ptr m_local_addr;
        callback m_callback; // 接收到注册中心的推送后需要执行的方法，由channel进行绑定
    };
}

#endif //RPCFRAME_RPC_PUBLISH_LISTENER_H
