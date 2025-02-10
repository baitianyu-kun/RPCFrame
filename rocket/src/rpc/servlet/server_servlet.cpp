//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/server_servlet.h"
#include "common/log.h"

namespace rocket {

    void ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }

    void RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }
}