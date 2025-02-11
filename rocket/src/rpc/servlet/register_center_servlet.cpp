//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/register_center_servlet.h"
#include "common/log.h"

namespace rocket {

    // 这里再去调用REGISTER CENTER里面的具体业务方法
    // GetRegisterCenter().update(params);
    // update进行加锁
    void ServerRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
        response->m_response_body = "ServerRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }

    void ClientRegisterServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
        std::string body_str;
//        body_str += "result:Success_handle_ClientRegisterServlet" + g_CRLF;
        body_str += "msg_id:1234";
        response->m_response_properties.setKeyValue("Content-Length",std::to_string(body_str.length()));
        response->m_response_body = body_str;
        DEBUGLOG("%s", body_str.c_str());
    }
}