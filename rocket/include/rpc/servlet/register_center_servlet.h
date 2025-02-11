//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_REGISTER_CENTER_SERVLET_H
#define RPCFRAME_REGISTER_CENTER_SERVLET_H

#include "net/protocol/http/http_servlet.h"

namespace rocket {

    // DispatcherServlet是单例
    // RegisterCenter也是单例，所以操作Register center的时候如果有update方法就需要加写锁，因为多个线程共同访问一个Register center，维护一个cache buffer
    class ServerRegisterServlet : public Servlet {
    public:
        using ptr = std::shared_ptr<ServerRegisterServlet>;

        ServerRegisterServlet() : Servlet("ServerRegisterServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;
    };

    class ClientRegisterServlet : public Servlet {
    public:
        using ptr = std::shared_ptr<ClientRegisterServlet>;

        ClientRegisterServlet() : Servlet("ClientRegisterServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;
    };
}

#endif //RPCFRAME_REGISTER_CENTER_SERVLET_H
