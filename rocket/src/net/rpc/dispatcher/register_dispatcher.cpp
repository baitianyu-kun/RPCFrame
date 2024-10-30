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
                // 添加过该方法的情况，自动更新
                method_full_name_find->second.emplace(server_addr); // 会自动去重
                m_method_balance[method_full_name]->AddNewPhysicalNode(server_addr->toString(),
                                                                       VIRTUAL_NODE_NUM); // 同样会自动去重
            } else {
                // 没有添加过该方法的情况，则创建
                std::set<NetAddr::net_addr_sptr_t_, CompNetAddr> tmp_peer_set;
                tmp_peer_set.emplace(server_addr);
                m_method_server.emplace(method_full_name, tmp_peer_set);
                // 为该方法添加balance
                ConsistentHash::con_hash_sptr_t_ con_hash = std::make_shared<ConsistentHash>();
                con_hash->AddNewPhysicalNode(server_addr->toString(), VIRTUAL_NODE_NUM);
                m_method_balance.emplace(method_full_name, con_hash);
            }
        }
        DEBUGLOG(printAllMethodServer().c_str());
        // DEBUGLOG(printAllMethodBalance().c_str());
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
            final_res = "add_method_count:" + std::to_string(method_full_name_vec.size()) + g_CRLF + "msg_id:" +
                        req_protocol->m_msg_id;
        } else {
            // 客户端传过来只能是一个method full name，即只能调一个，所以这里直接取第一个
            // 在dispatcher中查找该method对应的server
            // 这里是通过客户端的ip，来在负载均衡中找到合适的服务器去请求*method_full_name_vec.begin()这个方法
            auto server_find_balance = m_method_balance.find(*method_full_name_vec.begin());
            if (server_find_balance != m_method_balance.end()) {
                auto server_info_balance = server_find_balance->second->GetServer(peer_addr->toString());
                final_res = "success:" + std::to_string(true) + g_CRLF
                            + "server_ip:" + server_info_balance->getStringIP() + g_CRLF
                            + "server_port:" + server_info_balance->getStringPort() + g_CRLF
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

    std::string RegisterDispatcher::printAllMethodBalance() {
        std::string all_method_server = "In Balance: ";
        for (const auto &item: m_method_balance) {
            all_method_server = all_method_server + item.second->printAllServerNodes();
        }
        return all_method_server;
    }

    std::string RegisterDispatcher::printAllMethodServer() {
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
        DEBUGLOG("delete before: %s", printAllMethodServer().c_str());
        // DEBUGLOG("delete before: %s",printAllMethodBalance().c_str());
        auto iter = m_method_server.begin();
        for (; iter != m_method_server.end();) {
            // 该method对应的server list中进行删除
            iter->second.erase(server_addr);
            // 该method对应的balance中的server addr进行删除
            m_method_balance[iter->first]->DeletePhysicalNode(server_addr->toString());
            // 该method的所有服务器都为空的话，则删掉其
            if (iter->second.empty()) {
                m_method_balance.erase(iter->first); // 也删掉该method name下的所有负载均衡
                iter = m_method_server.erase(iter);
            } else {
                iter++;
            }
        }
        DEBUGLOG("delete after: %s", printAllMethodServer().c_str());
        // DEBUGLOG("delete after: %s",printAllMethodBalance().c_str());
    }

}



