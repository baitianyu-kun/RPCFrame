//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_SERVER_SERVLET_H
#define RPCFRAME_SERVER_SERVLET_H

#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "net/protocol/http/http_servlet.h"

namespace rocket {
    class ClientServerServlet : public Servlet {
    public:
        using ptr = std::shared_ptr<ClientServerServlet>;


        ClientServerServlet() : Servlet("ClientServerServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;

    };

    class RegisterUpdateServer : public Servlet {
    public:
        using ptr = std::shared_ptr<RegisterUpdateServer>;

        RegisterUpdateServer() : Servlet("RegisterUpdateServer") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) override;
    };
}

#endif //RPCFRAME_SERVER_SERVLET_H
