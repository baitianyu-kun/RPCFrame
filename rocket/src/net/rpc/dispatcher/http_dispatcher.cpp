//
// Created by baitianyu on 8/10/24.
//
#include <memory>
#include "net/rpc/dispatcher/http_dispatcher.h"
#include "common/string_util.h"
#include "common/log.h"
#include "common/runtime.h"
#include "net/rpc/rpc_controller.h"


namespace rocket {

    rocket::HTTPDispatcher::~HTTPDispatcher() {
        DEBUGLOG("~HTTPDispatcher");
    }

    // 检测到特定字段去执行向注册中心更新method list的操作
    // 这个应该抽象出一个基本方法来，可以处理rpc操作和其他操作，这里先简单的往上堆
    // 注册中心请求服务端: update, msg_id 服务端返回：update, method_full_name，msg_id, server_ip, server_port
    void HTTPDispatcher::dispatchUpdateRegister(const std::shared_ptr<HTTPRequest> &req_protocol,
                                                const std::shared_ptr<HTTPResponse> &rsp_protocol,
                                                NetAddr::net_addr_sptr_t_ peer_addr,
                                                NetAddr::net_addr_sptr_t_ local_addr) {
        auto all_service_names = getAllServiceName();
        std::string all_service_names_str = "";
        for (const auto &item: all_service_names) {
            all_service_names_str += item;
            all_service_names_str += ",";
        }
        all_service_names_str = all_service_names_str.substr(0, all_service_names_str.length() - 1);

        std::string msg_id = req_protocol->m_msg_id;
        std::string final_res = "update:" + std::to_string(true) + g_CRLF
                                + "server_ip:" + local_addr->getStringIP() + g_CRLF
                                + "server_port:" + local_addr->getStringPort() + g_CRLF
                                + "method_full_name:" + all_service_names_str + g_CRLF
                                + "msg_id:" + msg_id;

        rsp_protocol->m_response_body = final_res;
        rsp_protocol->m_response_version = req_protocol->m_request_version;
        rsp_protocol->m_response_code = HTTPCode::HTTP_OK;
        rsp_protocol->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        rsp_protocol->m_response_properties.m_map_properties["Content-Length"] = std::to_string(final_res.length());
        rsp_protocol->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        rsp_protocol->m_msg_id = msg_id;

        INFOLOG("%s | update server info to register dispatch success, req_protocol_body [%s], rsp_protocol_body [%s]",
                msg_id.c_str(), req_protocol->m_request_body.c_str(),
                rsp_protocol->m_response_body.c_str());
    }


    // response_body_map["msg_id"]和req_protocol_register_center->m_msg_id里面的msg id实际上都相等
    // 为了方面每次请求和返回时候都带上msg id
    // dispatcher中处理response时候，response的msg id和request的msg id得相等
    void HTTPDispatcher::dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                                  const AbstractProtocol::abstract_pro_sptr_t_ &response,
                                  NetAddr::net_addr_sptr_t_ peer_addr, NetAddr::net_addr_sptr_t_ local_addr) {
        auto req_protocol = std::dynamic_pointer_cast<HTTPRequest>(request);
        auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(response);
        std::unordered_map<std::string, std::string> req_body_data_map;
        splitStrToMap(req_protocol->m_request_body,
                      g_CRLF,
                      ":", req_body_data_map);
        req_protocol->m_msg_id = req_body_data_map["msg_id"];

        if (req_body_data_map.find("update") != req_body_data_map.end()) {
            dispatchUpdateRegister(req_protocol, rsp_protocol, peer_addr, local_addr);
            return;
        }

        // 请求体数据格式: method_full_name:Order.makeOrder'\r\n'pb_data:......
        auto method_full_name = req_body_data_map["method_full_name"];
        auto pb_data = req_body_data_map["pb_data"];

        std::string service_name;
        std::string method_name;
        if (!parseServiceFullName(method_full_name, service_name, method_name)) {
            // 做测试返回空网页用
            rsp_protocol->m_response_body = default_html_template;
            rsp_protocol->m_response_version = req_protocol->m_request_version;
            rsp_protocol->m_response_code = HTTPCode::HTTP_OK;
            rsp_protocol->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
            rsp_protocol->m_response_properties.m_map_properties["Content-Length"] = std::to_string(
                    strlen(default_html_template));
            rsp_protocol->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
            rsp_protocol->m_msg_id = req_protocol->m_msg_id;
            return;
        }

        auto iter = m_service_map.find(service_name);
        if (iter == m_service_map.end()) {
            ERRORLOG("%s | service name [%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
            return;
        }

        auto service = (*iter).second;
        auto method = service->GetDescriptor()->FindMethodByName(method_name);
        if (method == nullptr) {
            ERRORLOG("%s | method name [%s] not found in service[%s]", req_protocol->m_msg_id.c_str(),
                     method_name.c_str(), service_name.c_str());
            return;
        }

        auto req_msg = std::shared_ptr<google::protobuf::Message>(service->GetRequestPrototype(method).New());
        if (!req_msg->ParseFromString(pb_data)) {
            ERRORLOG("%s | deserialize error", req_protocol->m_msg_id.c_str(), method_name.c_str(),
                     service_name.c_str());
            return;
        }

        INFOLOG("%s | get rpc request[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());

        auto rsp_msg = std::shared_ptr<google::protobuf::Message>(service->GetResponsePrototype(method).New());
        auto rpc_controller = std::make_shared<RPCController>();
        rpc_controller->SetLocalAddr(local_addr);
        rpc_controller->SetPeerAddr(peer_addr);
        rpc_controller->SetMsgId(req_protocol->m_msg_id);

        RunTime::GetRunTime()->m_msg_id = req_protocol->m_msg_id;
        RunTime::GetRunTime()->m_method_full_name = method_full_name;

        service->CallMethod(method, rpc_controller.get(), req_msg.get(), rsp_msg.get(), nullptr);

        std::string res_pb_data;
        rsp_msg->SerializeToString(&res_pb_data);
        std::string final_res = "method_full_name:" + method_full_name + g_CRLF
                                + "pb_data:" + res_pb_data + g_CRLF
                                + "msg_id:" + req_protocol->m_msg_id;
        rsp_protocol->m_response_body = final_res;
        rsp_protocol->m_response_version = req_protocol->m_request_version;
        rsp_protocol->m_response_code = HTTPCode::HTTP_OK;
        rsp_protocol->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        rsp_protocol->m_response_properties.m_map_properties["Content-Length"] = std::to_string(final_res.length());
        rsp_protocol->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        rsp_protocol->m_msg_id = req_protocol->m_msg_id;
        INFOLOG("%s | http dispatch success, request[%s], response[%s]",
                req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str(),
                rsp_msg->ShortDebugString().c_str());
    }

    bool HTTPDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name,
                                              std::string &method_name) {
        if (full_name.empty()) {
            ERRORLOG("full name empty");
            return false;
        }
        // Order.makeOrder
        auto i = full_name.find_first_of(".");
        // find函数在找不到指定值得情况下会返回string::npos
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

    // 服务端注册时候注册的是：Order
    // 客户端来的时候是：Order.makeOrder
    void HTTPDispatcher::registerService(const HTTPDispatcher::protobuf_service_sptr_t_ &service) {
        auto service_name = service->GetDescriptor()->full_name();
        m_service_map[service_name] = service;
    }

    std::vector<std::string> HTTPDispatcher::getAllServiceName() {
        std::vector<std::string> tmp_service_names;
        for (const auto &item: m_service_map) {
            tmp_service_names.push_back(item.first);
        }
        return tmp_service_names;
    }

}



