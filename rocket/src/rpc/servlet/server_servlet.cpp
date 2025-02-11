//
// Created by baitianyu on 25-2-10.
//
#include "rpc/servlet/server_servlet.h"
#include "rpc/rpc_controller.h"
#include "common/log.h"

namespace rocket {

    void ClientServerServlet::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        // 处理具体业务
        auto method_full_name = request->m_request_body_data_map["method_full_name"];
        auto pb_data = request->m_request_body_data_map["pb_data"];
        std::string service_name;
        std::string method_name;
        if (!parseServiceFullName(method_full_name, service_name, method_name)) {
            // 做测试返回空网页用
            HTTPManager::copy(response, HTTPManager::createDefaultResponse());
            return;
        }

        auto iter = m_service_maps.find(service_name);
        if (iter == m_service_maps.end()) {
            ERRORLOG("%s | service name [%s] not found", request->m_msg_id.c_str(), service_name.c_str());
            return;
        }

        auto service = (*iter).second;
        auto method = service->GetDescriptor()->FindMethodByName(method_name);
        if (method == nullptr) {
            ERRORLOG("%s | method name [%s] not found in service [%s]", request->m_msg_id.c_str(),
                     method_name.c_str(), service_name.c_str());
            return;
        }

        auto request_rpc_message = std::shared_ptr<google::protobuf::Message>(
                service->GetRequestPrototype(method).New());
        if (!request_rpc_message->ParseFromString(pb_data)) {
            ERRORLOG("%s | deserialize error", request->m_msg_id.c_str(), method_name.c_str(),
                     service_name.c_str());
            return;
        }

        INFOLOG("%s | get rpc request [%s]", request->m_msg_id.c_str(), method_full_name.c_str());

        auto response_rpc_message = std::shared_ptr<google::protobuf::Message>(
                service->GetResponsePrototype(method).New());

        auto controller = std::make_shared<RPCController>();
        controller->SetLocalAddr();
        controller->SetPeerAddr();
        controller->SetMsgId();


    }

    void ClientServerServlet::addService(const ClientServerServlet::protobuf_service_ptr &service) {
        auto service_name = service->GetDescriptor()->full_name();
        m_service_maps[service_name] = service;
    }

    std::vector<std::string> ClientServerServlet::getAllServiceNames() {
        std::vector<std::string> tmp_service_names;
        for (const auto &item: m_service_maps) {
            tmp_service_names.push_back(item.first);
        }
        return tmp_service_names;
    }

    std::string ClientServerServlet::getAllServiceNamesStr() {
        std::string all_service_names_str = "";
        for (const auto &item: getAllServiceNames()) {
            all_service_names_str += item;
            all_service_names_str += ",";
        }
        return all_service_names_str;
    }

    bool ClientServerServlet::parseServiceFullName(const std::string &full_name, std::string &service_name,
                                                   std::string &method_name) {
        if (full_name.empty()) {
            ERRORLOG("full name empty");
            return false;
        }
        // Order.makeOrder
        auto i = full_name.find_first_of(".");
        if (i == full_name.npos) {
            ERRORLOG("not find '.' in full name [%s]", full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0, i);
        method_name = full_name.substr(i + 1, full_name.length() - i - 1);
        INFOLOG("parse service_name [%s] and method_name [%s] from full name [%s]", service_name.c_str(),
                method_name.c_str(), full_name.c_str());
        return true;
    }

    void RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        response->m_response_body = "RegisterUpdateServer::handle(HTTPRequest::ptr request, HTTPResponse::ptr response)";
    }
}