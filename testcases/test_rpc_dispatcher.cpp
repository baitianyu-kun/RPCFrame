//
// Created by baitianyu on 2/8/25.
//
#include "rpc/rpc_dispatcher.h"
#include "net/protocol/http/http_parse.h"
#include "common/log.h"
#include "event/io_thread_pool.h"

using namespace rocket;

int main() {

    Config::SetGlobalConfig("../conf/rocket.xml");
    Logger::InitGlobalLogger(0);

    RPCDispatcher::body_type body;
    body["server_ip"] = "192.168.1.1";
    body["server_port"] = "22222";
    body["method_full_name"] = "order";
    body["pb_data"] = "prices";
    body["all_method_full_names"] = "all_method_full_names";
    auto res1 = RPCDispatcher::createRequest(RPCDispatcher::MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, body);
    auto res2 = RPCDispatcher::createRequest(RPCDispatcher::MSGType::RPC_REGISTER_UPDATE_SERVER_REQUEST, body);
    auto res3 = RPCDispatcher::createRequest(RPCDispatcher::MSGType::RPC_METHOD_REQUEST, body);
    auto res4 = RPCDispatcher::createRequest(RPCDispatcher::MSGType::RPC_SERVER_REGISTER_REQUEST, body);

    DEBUGLOG("%s", res1->toString().c_str());
    DEBUGLOG("%s", res2->toString().c_str());
    DEBUGLOG("%s", res3->toString().c_str());
    DEBUGLOG("%s", res4->toString().c_str());


    DEBUGLOG("");
    DEBUGLOG("");

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

    for (auto item: strs) {
        HTTPRequestParser requestParser;
        requestParser.parse(item);
        auto res = std::make_shared<HTTPResponse>();
        RPCDispatcher::GetCurrentRPCDispatcher()->handle(requestParser.getRequest(), res);
    }
    return 0;
}