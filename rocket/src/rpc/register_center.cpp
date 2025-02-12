//
// Created by baitianyu on 25-2-10.
//
#include "rpc/register_center.h"
#include "common/mutex.h"
#include "common/string_util.h"

namespace rocket {

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
    RegisterCenter::updateServiceServer(std::vector<std::string> all_method_full_names_vec, NetAddr::ptr server_addr) {
        for (const auto &method_full_name: all_method_full_names_vec) {
            m_service_servers[method_full_name].emplace(server_addr);
        }
        m_servers_service.emplace(server_addr, all_method_full_names_vec);
    }

    void RegisterCenter::handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                              HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
        auto all_method_full_names = request->m_request_body_data_map["all_method_full_names"];
        std::vector<std::string> all_method_full_names_vec;
        splitStrToVector(all_method_full_names, ",", all_method_full_names_vec);
        auto server_addr = std::make_shared<IPNetAddr>(request->m_request_body_data_map["server_ip"],
                                                       std::stoi(request->m_request_body_data_map["server_port"]));
        updateServiceServer(all_method_full_names_vec, server_addr);
        HTTPManager::body_type body;
        body["add_service_count"] = std::to_string(all_method_full_names_vec.size());
        body["msg_id"] = request->m_msg_id;
        HTTPManager::createResponse(response, HTTPManager::MSGType::RPC_SERVER_REGISTER_RESPONSE, body);
        INFOLOG("%s | server register success, server addr [%s], services [%s]", request->m_msg_id.c_str(),
                server_addr->toString().c_str(), getAllServiceNamesStr().c_str());
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

    void RegisterCenter::notifyClientServerFailed() {

    }

    void RegisterCenter::publishClientMessage() {

    }
}

