//
// Created by baitianyu on 2/8/25.
//
#include "net/protocol/http/http_define.h"
#include "net/protocol/http/http_parse.h"
#include "net/protocol/http/http_servlet.h"
#include "common/config.h"
#include "common/log.h"
#include <iostream>

using namespace mrpc;

int main() {

    Config::SetGlobalConfig("../conf/mrpc.xml");
    Logger::InitGlobalLogger(0);

    auto dispatcher = std::make_shared<DispatchServlet>();

    Servlet::ptr call_back_servlet = std::make_shared<CallBacksServlet>(
            [](HTTPRequest::ptr reqeust, HTTPResponse::ptr response, HTTPSession::ptr session) {
                DEBUGLOG("call back dispatcher success")
                std::string str2 = "HTTP/1.1 200 OK\r\n"
                                   "Server: Apache-Coyote/1.1\r\n"
                                   "Content-Type: text/html;charset=UTF-8\r\n"
                                   "Content-Length: 30\r\n"
                                   "\r\n"
                                   "username=zhangsan&password=123";
                HTTPResponseParser responseParser;
                responseParser.parse(str2);
                auto res = responseParser.getResponse();
                response->m_response_code = res->m_response_code;
                response->m_response_properties = res->m_response_properties;
                response->m_response_info = res->m_response_info;
                response->m_response_version = res->m_response_version;
                response->m_response_body = res->m_response_body;
            });

    dispatcher->addServlet("/hello", call_back_servlet);

    std::string str = "POST /hello HTTP/1.0\r\n"
                      "Host: 10.101.101.10\r\n"
                      "Content-Length: 30\r\n"
                      "Content-Type: text/html;charset=utf-8\r\n"
                      "\r\n"
                      "username=zhangsan&password=123";
    HTTPRequestParser requestParser;
    requestParser.parse(str);

    auto res = std::make_shared<HTTPResponse>();

    IPNetAddr::ptr local_addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 22224);
    IPNetAddr::ptr peer_addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 22225);
    auto session = std::make_shared<HTTPSession>(local_addr, peer_addr);
    dispatcher->handle(requestParser.getRequest(), res, session);

    DEBUGLOG("success get res");
    DEBUGLOG("%s", res->toString().c_str());

    return 0;
}