//
// Created by baitianyu on 25-2-10.
//
#include <unistd.h>
#include "rpc/register_center.h"
#include "common/mutex.h"
#include "common/string_util.h"

namespace mrpc {

    RegisterCenter::RegisterCenter(NetAddr::ptr local_addr) : TCPServer(local_addr), m_local_addr(local_addr) {
        initServlet();
    }

    RegisterCenter::~RegisterCenter() {
        DEBUGLOG("~RegisterCenter");
    }

    void RegisterCenter::initServlet() {
        // 客户端访问注册中心
        addServlet(RPC_CLIENT_REGISTER_DISCOVERY_PATH, std::bind(&RegisterCenter::handleClientDiscovery, this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3));
        // 服务端访问注册中心
        addServlet(RPC_SERVER_REGISTER_PATH, std::bind(&RegisterCenter::handleServerRegister, this,
                                                       std::placeholders::_1,
                                                       std::placeholders::_2,
                                                       std::placeholders::_3));
        // 客户端订阅注册中心
        addServlet(RPC_REGISTER_SUBSCRIBE_PATH, std::bind(&RegisterCenter::handleClientSubscribe, this,
                                                          std::placeholders::_1,
                                                          std::placeholders::_2,
                                                          std::placeholders::_3));
        // 服务器发送心跳包给注册中心
        addServlet(RPC_REGISTER_HEART_SERVER_PATH, std::bind(&RegisterCenter::handleServerHeart, this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3));
    }

    void RegisterCenter::startRegisterCenter() {
        start();
    }

    std::string RegisterCenter::getAllServiceNamesStr() {
        std::string tmp = "";
        for (const auto &item: m_service_servers) {
            tmp += item.first + ",";
        }
        return tmp.substr(0, tmp.size() - 1);
    }

    void
    RegisterCenter::updateServiceServer(std::vector<std::string> all_services_names_vec, NetAddr::ptr server_addr) {
        for (const auto &service: all_services_names_vec) {
            m_service_servers[service].emplace(server_addr);
        }
        m_servers_service.emplace(server_addr->toString(), all_services_names_vec);
    }

    void RegisterCenter::handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                              HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
        auto all_services_names = request->m_request_body_data_map["all_services_names"];
        std::vector<std::string> all_services_names_vec;
        splitStrToVector(all_services_names, ",", all_services_names_vec);
        auto server_addr = std::make_shared<IPNetAddr>(request->m_request_body_data_map["server_ip"],
                                                       std::stoi(request->m_request_body_data_map["server_port"]));
        updateServiceServer(all_services_names_vec, server_addr);
        HTTPManager::body_type body;
        body["add_service_count"] = std::to_string(all_services_names_vec.size());
        body["msg_id"] = request->m_msg_id;
        HTTPManager::createResponse(response, HTTPManager::MSGType::RPC_SERVER_REGISTER_RESPONSE, body);
        INFOLOG("%s | server register success, server addr [%s], services [%s]", request->m_msg_id.c_str(),
                server_addr->toString().c_str(), getAllServiceNamesStr().c_str());
        // 给该服务器设置心跳定时器，每次server创建的client的端口号都不尽相同，所以需要rpc server在请求体中指明具体地址
        if (m_servers_timer_event.find(server_addr->toString()) == m_servers_timer_event.end()) {
            Timestamp timestamp(addTime(Timestamp::now(), 5));
            auto new_timer_id = getMainEventLoop()->addTimerEvent([server_addr, this]() {
                this->serverTimeOut(server_addr);
            }, timestamp, 0);
            m_servers_timer_event.emplace(server_addr->toString(), new_timer_id);
        }
    }

    void RegisterCenter::handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                               HTTPSession::ptr session) {
        RWMutex::ReadLock lock(m_mutex);
        auto service_name = request->m_request_body_data_map["service_name"];
        HTTPManager::body_type body;
        auto find = m_service_servers.find(service_name);
        if (find == m_service_servers.end()) {
            // 没有提供这个服务的server list
        } else {
            auto server_list = find->second;
            std::string server_list_str;
            for (const auto &item: server_list) {
                server_list_str += item->toString() + ",";
            }
            server_list_str = server_list_str.substr(0, server_list_str.size() - 1);
            body["server_list"] = server_list_str;
            body["msg_id"] = request->m_msg_id;
            HTTPManager::createResponse(response, HTTPManager::MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE, body);
            INFOLOG("%s | service discovery success, peer addr [%s], server_lists {%s}", request->m_msg_id.c_str(),
                    session->getPeerAddr()->toString().c_str(), server_list_str.c_str());
        }
    }

    void RegisterCenter::handleClientSubscribe(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                               HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
        // TODO 加入不存在该service name的情况，因为注册中心中可能不包含要注册的service
        auto service_name = request->m_request_body_data_map["service_name"];
        m_service_clients[service_name].emplace(session->getPeerAddr());
        HTTPManager::body_type body;
        body["service_name"] = service_name;
        body["msg_id"] = request->m_msg_id;
        body["subscribe_success"] = std::to_string(true);
        HTTPManager::createResponse(response, HTTPManager::MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE, body);
        INFOLOG("%s | service subscribe success, peer addr [%s], service_name {%s}", request->m_msg_id.c_str(),
                session->getPeerAddr()->toString().c_str(), service_name.c_str());
    }

    void
    RegisterCenter::handleServerHeart(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
        // 收到心跳包后重置这个server的定时器，然后回一个心跳包
        auto server_addr = std::make_shared<IPNetAddr>(request->m_request_body_data_map["server_ip"],
                                                       std::stoi(request->m_request_body_data_map["server_port"]));
        auto server_addr_str = server_addr->toString();
        getMainEventLoop()->deleteTimerEvent(m_servers_timer_event[server_addr_str]);

        Timestamp timestamp(addTime(Timestamp::now(), 5)); // 表示事件5秒后到达
        auto new_timer_id = getMainEventLoop()->addTimerEvent([server_addr, this]() {
            this->serverTimeOut(server_addr);
        }, timestamp, 0.0); // 如果是重复事件则设置间隔
        m_servers_timer_event[server_addr_str] = new_timer_id;

        HTTPManager::body_type body;
        body["msg_id"] = request->m_msg_id;
        HTTPManager::createResponse(response, HTTPManager::MSGType::RPC_REGISTER_HEART_SERVER_RESPONSE, body);
        INFOLOG("%s | receive peer addr [%s] heart pack", request->m_msg_id.c_str(), server_addr_str.c_str());
    }

    void RegisterCenter::serverTimeOut(NetAddr::ptr server_addr) {
        RWMutex::WriteLock lock(m_mutex);
        auto server_addr_str = server_addr->toString();
        if (m_servers_timer_event.find(server_addr_str) != m_servers_timer_event.end()) {
            DEBUGLOG("===== server [%s] time out ======", server_addr_str.c_str());
            getMainEventLoop()->deleteTimerEvent(m_servers_timer_event[server_addr_str]);
            m_servers_timer_event.erase(server_addr_str);
        }
    }

    void RegisterCenter::notifyClientServerFailed() {

    }

    void RegisterCenter::publishClientMessage() {
        auto io_thread = std::make_unique<IOThread>();
        auto client = std::make_shared<TCPClient>(*m_service_clients["Order"].begin(),
                                                  io_thread->getEventLoop());
        auto request = std::make_shared<HTTPRequest>();
        HTTPManager::body_type body;
        std::string service_name = "Order";
        body["service_name"] = service_name;
        HTTPManager::createRequest(request, HTTPManager::MSGType::RPC_REGISTER_CLIENT_PUBLISH_REQUEST, body);
        client->connect([client, request, service_name]() {
            client->sendRequest(request, [client](HTTPRequest::ptr req) {
                INFOLOG("%s | publish message to peer addr %s", req->m_msg_id.c_str(),
                        client->getPeerAddr()->toString().c_str());
            });
            client->recvResponse(request->m_msg_id,
                                 [client, request, service_name](HTTPResponse::ptr rsp) {
                                     client->getEventLoop()->stop();
                                     INFOLOG("%s | success publish peer addr %s", rsp->m_msg_id.c_str(),
                                             client->getPeerAddr()->toString().c_str());
                                 });
        });
        io_thread->start();
    }
}

