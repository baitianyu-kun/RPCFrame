//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/register_center_servlet.h"
#include "common/log.h"

namespace rocket {

    // 这里再去调用REGISTER CENTER里面的具体业务方法
    // GetRegisterCenter().update(params);
    // update进行加锁
    void ServerRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "ServerRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }

    void ClientRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "ClientRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }
}