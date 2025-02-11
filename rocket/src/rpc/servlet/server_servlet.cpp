//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/server_servlet.h"
#include "rpc/rpc_server.h"
#include "common/log.h"

namespace rocket {

    void ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
        RPCServer::GetRPCServerPtr()->handle(request, response, session);
    }

    void RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
        response->m_response_body = "RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }
}