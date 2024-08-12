//
// Created by baitianyu on 8/11/24.
//
#include <algorithm>
#include "net/rpc/dispatcher/register_dispatcher.h"
#include "net/coder/http/http_request.h"
#include "net/coder/http/http_response.h"
#include "common/string_util.h"
#include "common/log.h"

namespace rocket {

    RegisterDispatcher::~RegisterDispatcher() {
        DEBUGLOG("~RegisterDispatcher");
    }

    void RegisterDispatcher::updateMethodServer(std::vector<std::string> method_full_name_vec,
                                                NetAddr::net_addr_sptr_t_ server_addr) {
        for (const auto &method_full_name: method_full_name_vec) {
            // 在dispatcher中注册server和其所有的method
            // std::unordered_map<std::string, std::set<NetAddr::net_addr_sptr_t_>>
            auto method_full_name_find = m_method_server.find(method_full_name);
            if (method_full_name_find != m_method_server.end()) {
                // 目前因为这里是set，所以可能会出现新的添加后，旧的没办法访问，但是又删不掉的情况，需要后面做修改
                // 只能是定时任务里面添加后，定时清除掉time out的peer addr
                auto find_server_iter = std::find_if(method_full_name_find->second.begin(),
                                                     method_full_name_find->second.end(),
                                                     [&server_addr](const NetAddr::net_addr_sptr_t_ &x) {
                                                         return x->toString() == server_addr->toString();
                                                     });
                if (find_server_iter != method_full_name_find->second.end()) {
                    *find_server_iter = server_addr; // 旧的覆盖掉
                } else {
                    method_full_name_find->second.emplace_back(server_addr);
                }
            } else {
                std::vector<NetAddr::net_addr_sptr_t_> tmp_peer_vec;
                tmp_peer_vec.emplace_back(server_addr);
                m_method_server.emplace(method_full_name, tmp_peer_vec);
            }
        }
        DEBUGLOG(printAllMethodServer().c_str());
    }

    // 为了方面每次请求和返回时候都带上msg id
    // dispatcher中处理response时候，response的msg id和request的msg id得相等
    void RegisterDispatcher::dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                                      const AbstractProtocol::abstract_pro_sptr_t_ &response,
                                      NetAddr::net_addr_sptr_t_ peer_addr, NetAddr::net_addr_sptr_t_ local_addr) {
        // 从消息体里面携带的数据来判断是哪边的dispatch
        // 请求体字段：method_full_name, is_server
        // 接受服务端的请求，并在dispatcher中注册server和其所有的method
        // 接受客户端的请求，并在dispatcher中查找该method对应的server(后期可加负载均衡)
        auto req_protocol = std::dynamic_pointer_cast<HTTPRequest>(request);
        auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(response);
        std::string final_res;
        std::unordered_map<std::string, std::string> req_body_data_map;
        splitStrToMap(req_protocol->m_request_body,
                      g_CRLF,
                      ":", req_body_data_map);
        auto method_full_name_all = req_body_data_map["method_full_name"];
        // method full name: Name:1,2,3\r\nAge:1,2,4，可能会存在多个的情况
        std::vector<std::string> method_full_name_vec;
        splitStrToVector(method_full_name_all, ",", method_full_name_vec);
        req_protocol->m_msg_id = req_body_data_map["msg_id"];
        // 这个字段为1是代表为server，为0是代表为client
        auto is_server_find = req_body_data_map.find("is_server");
        auto is_server = false;
        if (is_server_find != req_body_data_map.end() && is_server_find->second == std::to_string(true)) {
            is_server = true;
        }
        if (is_server) {
            IPNetAddr::net_addr_sptr_t_ server_addr = std::make_shared<IPNetAddr>(req_body_data_map["server_ip"],
                                                                                  std::stoi(
                                                                                          req_body_data_map["server_port"]));
            updateMethodServer(method_full_name_vec, server_addr);
            final_res = "add_count:" + std::to_string(method_full_name_vec.size()) + g_CRLF + "msg_id:" +
                        req_protocol->m_msg_id;
        } else {
            // 客户端传过来只能是一个method full name，即只能调一个，所以这里直接取第一个
            // 在dispatcher中查找该method对应的server(后期可加负载均衡)
            auto server_find = m_method_server.find(*method_full_name_vec.begin());
            if (server_find != m_method_server.end()) {
                final_res = "success:" + std::to_string(true) + g_CRLF
                            + "server_ip:" + server_find->second.begin()->get()->getStringIP() + g_CRLF
                            + "server_port:" + server_find->second.begin()->get()->getStringPort() + g_CRLF
                            + "msg_id:" + req_protocol->m_msg_id;
            } else {
                final_res = "success:" + std::to_string(false) + g_CRLF
                            + "msg_id:" + req_protocol->m_msg_id;
            }
        }
        rsp_protocol->m_response_body = final_res;
        rsp_protocol->m_response_version = req_protocol->m_request_version;
        rsp_protocol->m_response_code = HTTPCode::HTTP_OK;
        rsp_protocol->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        rsp_protocol->m_response_properties.m_map_properties["Content-Length"] = std::to_string(final_res.length());
        rsp_protocol->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        rsp_protocol->m_msg_id = req_protocol->m_msg_id;
        INFOLOG("%s | dispatch success, is_server [%s], final_res [%s]",
                req_protocol->m_msg_id.c_str(), std::to_string(is_server).c_str(),
                final_res.c_str());
    }

    std::set<NetAddr::net_addr_sptr_t_> RegisterDispatcher::getAllServerList() {
        std::set<NetAddr::net_addr_sptr_t_> tmp_server_list;
        for (const auto &i: m_method_server) {
            for (const auto &j: i.second) {
                tmp_server_list.emplace(j);
            }
        }
        return tmp_server_list;
    }

    std::string RegisterDispatcher::printAllMethodServer() {
        // 这里只print了第一个的addr
        std::string all_method_server = "In Register: ";
        for (const auto &item: m_method_server) {
            std::string all_server = "";
            for (const auto &server: item.second) {
                all_server = all_server + server->toString() + ",";
            }
            all_server = all_server.substr(0, all_server.length() - 1);
            all_method_server += all_server;
        }
        return all_method_server;
    }

    void RegisterDispatcher::deleteServerInServerList(NetAddr::net_addr_sptr_t_ server_addr) {
        auto iter = m_method_server.begin();
        for (; iter != m_method_server.end();) {
            auto find_server_iter = std::find_if(iter->second.begin(), iter->second.end(),
                                                 [&server_addr](const NetAddr::net_addr_sptr_t_ &x) {
                                                     return x->toString() == server_addr->toString();
                                                 });
            if (find_server_iter != iter->second.end()) {
                iter->second.erase(find_server_iter);
            }
            if (iter->second.empty()) {
                iter = m_method_server.erase(iter);
            } else {
                iter++;
            }
        }
    }

}



