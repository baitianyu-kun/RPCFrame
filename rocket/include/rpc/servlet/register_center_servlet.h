//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_REGISTER_CENTER_SERVLET_H
#define RPCFRAME_REGISTER_CENTER_SERVLET_H

#include "net/protocol/http/http_servlet.h"

namespace rocket {
    // 处理服务器注册到注册中心，注册中心执行业务并返回信息
    // 设置friend class Register，以此让Servlet执行Register中的业务
    // 多重继承？既让其为servlet，又让其为tcpserver和client
    class ServerRegisterServlet : public Servlet {
    public:
        using ptr = std::shared_ptr<ServerRegisterServlet>;

        ServerRegisterServlet() : Servlet("ServerRegisterServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response) override;
    };

    class ClientRegisterServlet : public Servlet {
    public:
        using ptr = std::shared_ptr<ClientRegisterServlet>;

        ClientRegisterServlet() : Servlet("ClientRegisterServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response) override;
    };
}

#endif //RPCFRAME_REGISTER_CENTER_SERVLET_H
