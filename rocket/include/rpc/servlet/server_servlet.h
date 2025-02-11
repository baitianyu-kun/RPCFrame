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

        using protobuf_service_ptr = std::shared_ptr<google::protobuf::Service>;

        ClientServerServlet() : Servlet("ClientServerServlet") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response) override;

        void addService(const protobuf_service_ptr &service);

        std::vector<std::string> getAllServiceNames();

        std::string getAllServiceNamesStr();

    private:
        static bool
        parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name);

    private:
        std::unordered_map<std::string, protobuf_service_ptr> m_service_maps;
    };

    class RegisterUpdateServer : public Servlet {
    public:
        using ptr = std::shared_ptr<RegisterUpdateServer>;

        RegisterUpdateServer() : Servlet("RegisterUpdateServer") {}

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response) override;
    };
}

#endif //RPCFRAME_SERVER_SERVLET_H
