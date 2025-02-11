//
// Created by baitianyu on 25-2-10.
//
#include "net/tcp/tcp_client.h"
#include "common/log.h"
#include "common/string_util.h"

using namespace rocket;

int main() {
    Config::SetGlobalConfig("../conf/rocket.xml");
    Logger::InitGlobalLogger(0);
    IPNetAddr::ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);

    auto client = std::make_shared<TCPClient>(addr);

    std::string request_str = "POST /method HTTP/1.0\r\n"
                              "Host: 10.101.101.10\r\n"
                              "Content-Length: 30\r\n"
                              "Content-Type: text/html;charset=utf-8\r\n"
                              "\r\n"
                              "msg_id:1234";
    HTTPRequestParser::ptr requestParser = std::make_shared<HTTPRequestParser>();
    requestParser->parse(request_str);
    auto request = requestParser->getRequest();

    auto msg_id = requestParser->getRequest()->m_msg_id;
    client->connect([client, request, msg_id]() {
        INFOLOG("%s | success connect. peer addr [%s], local addr[%s]",
                msg_id.c_str(),
                client->getPeerAddr()->toString().c_str(),
                client->getLocalAddr()->toString().c_str());

        client->sendRequest(request, [client, request, msg_id](HTTPRequest::ptr msg) {
            INFOLOG("%s | success send rpc request. call method name [%s], peer addr [%s], local addr[%s]",
                    msg_id.c_str(),
                    request->m_request_body.c_str(),
                    client->getPeerAddr()->toString().c_str(),
                    client->getLocalAddr()->toString().c_str());

            client->recvResponse(msg_id, [client, request](HTTPResponse::ptr msg) {
                INFOLOG("%s | success get rpc response, rsp_protocol_body [%s], peer addr [%s], local addr[%s], response [%s]",
                        msg->m_msg_id.c_str(), msg->m_response_body.c_str(),
                        client->getPeerAddr()->toString().c_str(),
                        client->getLocalAddr()->toString().c_str(),
                        msg->toString().c_str());
                client->getEventLoop()->stop();
            });
        });

    });
    return 0;
}