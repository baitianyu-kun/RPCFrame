//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/server_servlet.h"
#include "common/log.h"

namespace rocket {

    void ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }

    void ClientServerServlet::registerService(const ClientServerServlet::protobuf_service_ptr &service) {
        auto service_name = service->GetDescriptor()->full_name();
        m_service_maps[service_name] = service;
    }

    std::vector<std::string> ClientServerServlet::getAllServiceNames() {
        std::vector<std::string> tmp_service_names;
        for (const auto &item: m_service_maps) {
            tmp_service_names.push_back(item.first);
        }
        return tmp_service_names;
    }

    void RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }
}