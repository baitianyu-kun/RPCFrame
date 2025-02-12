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

    void
    RegisterCenter::updateServiceServer(std::vector<std::string> all_method_full_names_vec, NetAddr::ptr server_addr) {
        for (const auto &method_full_name: all_method_full_names_vec) {
            auto find = m_service_servers.find(method_full_name);
            if (find != m_service_servers.end()) {
                // 添加过该方法，进行更新
                find->second.emplace(server_addr);

            }
        }
    }

    void RegisterCenter::handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                              HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
        auto all_method_full_names = request->m_request_body_data_map["all_method_full_names"];
        std::vector<std::string> all_method_full_names_vec;
        splitStrToVector(all_method_full_names, ",", all_method_full_names_vec);
        auto server_addr = std::make_shared<IPNetAddr>(request->m_request_body_data_map["server_ip"],
                                                       std::stoi(request->m_request_body_data_map["server_port"]));


    }

    void RegisterCenter::handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                               HTTPSession::ptr session) {
        RWMutex::ReadLock lock(m_mutex);

    }

    void RegisterCenter::notifyClientServerFailed() {

    }

    void RegisterCenter::publishClientMessage() {

    }
}

