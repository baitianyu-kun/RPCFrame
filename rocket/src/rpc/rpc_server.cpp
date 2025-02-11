//
// Created by baitianyu on 2/10/25.
//
#include "rpc/rpc_server.h"
#include "net/protocol/http/http_define.h"

namespace rocket {

    RPCServer::RPCServer(NetAddr::ptr local_addr, NetAddr::ptr register_addr)
            : TCPServer(local_addr), m_local_addr(local_addr), m_register_addr(register_addr) {
        initServlet();
    }

    RPCServer::~RPCServer() {
        DEBUGLOG("~RPCServer");
    }

    void RPCServer::initServlet() {
        // 客户端访问服务器
        // 注册中心访问服务器
        m_client_server_servlet = std::make_shared<ClientServerServlet>();
        m_register_update_server_servlet = std::make_shared<RegisterUpdateServer>();
        addServlet(RPC_METHOD_PATH, m_client_server_servlet);
        addServlet(RPC_REGISTER_UPDATE_SERVER_PATH, m_register_update_server_servlet);
    }

    void RPCServer::registerToCenter() {
        auto &io_thread = getIOThreadPool()->getIOThread();
        // 放到线程里执行，因为TCPClient里面的TCPConnection用的eventloop和当前RPCServer是一个(因为都是主线程)，所以把这个
        // 函数放到子线程中去执行，就不是一个eventloop了，防止造成影响
        auto client = std::make_shared<TCPClient>(m_register_addr, io_thread->getEventLoop());
        HTTPManager::body_type body;
        body["server_ip"] = m_local_addr->getStringIP();
        body["server_port"] = m_local_addr->getStringPort();
        body["all_method_full_names"] = m_client_server_servlet->getAllServiceNamesStr();
        HTTPRequest::ptr request = HTTPManager::createRequest(HTTPManager::MSGType::RPC_SERVER_REGISTER_REQUEST, body);

        client->connect([&client, request]() {
            client->sendRequest(request, [&client, request](HTTPRequest::ptr msg) {
                client->recvResponse(request->m_msg_id, [&client, request](HTTPResponse::ptr msg) {
                    INFOLOG("%s | success register to center, rsp_protocol_body [%s], peer addr [%s], local addr[%s], response [%s]",
                            msg->m_msg_id.c_str(), msg->m_response_body.c_str(),
                            client->getPeerAddr()->toString().c_str(),
                            client->getLocalAddr()->toString().c_str(),
                            msg->toString().c_str());
                    client->getEventLoop()->stop();
                    client.reset();
                });
            });
        });
    }

    void RPCServer::addService(ClientServerServlet::protobuf_service_ptr service) {
        m_client_server_servlet->addService(service);
    }

    void RPCServer::startRPC() {
        start();
    }
}

