//
// Created by baitianyu on 2/8/25.
//
#include "rpc/rpc_dispatcher.h"
#include "net/protocol/http/http_parse.h"
#include "common/log.h"
#include "event/io_thread_pool.h"
#include "rpc/servlet/register_center_servlet.h"
#include "rpc/servlet/server_servlet.h"
#include "net/protocol/http/http_define.h"

using namespace rocket;

int main() {

    Config::SetGlobalConfig("../conf/rocket.xml");
    Logger::InitGlobalLogger(0);

    // 请求相应创建测试
//    HTTPManager::body_type body;
//    body["server_ip"] = "192.168.1.1";
//    body["server_port"] = "22222";
//    body["method_full_name"] = "order";
//    body["pb_data"] = "prices";
//    body["all_method_full_names"] = "all_method_full_names";
//    auto res1 = HTTPManager::createRequest(HTTPManager::MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, body);
//    auto res2 = HTTPManager::createRequest(HTTPManager::MSGType::RPC_REGISTER_UPDATE_SERVER_REQUEST, body);
//    auto res3 = HTTPManager::createRequest(HTTPManager::MSGType::RPC_METHOD_REQUEST, body);
//    auto res4 = HTTPManager::createRequest(HTTPManager::MSGType::RPC_SERVER_REGISTER_REQUEST, body);
//
//    DEBUGLOG("%s", res1->toString().c_str());
//    DEBUGLOG("%s", res2->toString().c_str());
//    DEBUGLOG("%s", res3->toString().c_str());
//    DEBUGLOG("%s", res4->toString().c_str());
//
//
//    DEBUGLOG("");
//    DEBUGLOG("");


    // dispatcher测试
    std::string str = "POST /method HTTP/1.0\r\n"
                      "Host: 10.101.101.10\r\n"
                      "Content-Length: 30\r\n"
                      "Content-Type: text/html;charset=utf-8\r\n"
                      "\r\n"
                      "username=zhangsan&password=123";

    std::string str2 = "POST /update HTTP/1.0\r\n"
                       "Host: 10.101.101.10\r\n"
                       "Content-Length: 30\r\n"
                       "Content-Type: text/html;charset=utf-8\r\n"
                       "\r\n"
                       "username=zhangsan&password=123";

    std::string str3 = "POST /register HTTP/1.0\r\n"
                       "Host: 10.101.101.10\r\n"
                       "Content-Length: 30\r\n"
                       "Content-Type: text/html;charset=utf-8\r\n"
                       "\r\n"
                       "username=zhangsan&password=123";

    std::string str4 = "POST /discovery HTTP/1.0\r\n"
                       "Host: 10.101.101.10\r\n"
                       "Content-Length: 30\r\n"
                       "Content-Type: text/html;charset=utf-8\r\n"
                       "\r\n"
                       "username=zhangsan&password=123";

    std::vector<std::string> strs;
    strs.emplace_back(str);
    strs.emplace_back(str2);
    strs.emplace_back(str3);
    strs.emplace_back(str4);

    auto s1 = std::make_shared<ServerRegisterServlet>();
    auto s2 = std::make_shared<ClientRegisterServlet>();
    auto s3 = std::make_shared<ClientServerServlet>();
    auto s4 = std::make_shared<RegisterUpdateServer>();

    RPCDispatcher::GetCurrentRPCDispatcher()->addServlet(RPC_SERVER_REGISTER_PATH, s1);
    RPCDispatcher::GetCurrentRPCDispatcher()->addServlet(RPC_CLIENT_REGISTER_DISCOVERY_PATH, s2);
    RPCDispatcher::GetCurrentRPCDispatcher()->addServlet(RPC_METHOD_PATH, s3);
    RPCDispatcher::GetCurrentRPCDispatcher()->addServlet(RPC_REGISTER_UPDATE_SERVER_PATH, s4);

    for (auto item: strs) {
        HTTPRequestParser requestParser;
        requestParser.parse(item);
        auto res = std::make_shared<HTTPResponse>();
        IPNetAddr::ptr local_addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
        IPNetAddr::ptr peer_addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22225);
        auto session = std::make_shared<HTTPSession>(local_addr, peer_addr);
        RPCDispatcher::GetCurrentRPCDispatcher()->handle(requestParser.getRequest(), res, session);
        DEBUGLOG("%s", res->m_response_body.c_str());
    }
    return 0;
}