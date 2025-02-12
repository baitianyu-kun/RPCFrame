//
// Created by baitianyu on 2/11/25.
//
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rpc/rpc_channel.h"
#include "rpc/rpc_controller.h"
#include "common/log.h"
#include "common/string_util.h"
#include "event/io_thread.h"

namespace rocket {

    RPCChannel::RPCChannel(NetAddr::ptr register_center_addr) : m_register_center_addr(register_center_addr) {

    }

    RPCChannel::~RPCChannel() {
        DEBUGLOG("~RPCChannel");
    }

    void RPCChannel::init(RPCChannel::google_rpc_controller_ptr controller, RPCChannel::google_message_ptr request,
                          RPCChannel::google_message_ptr response, RPCChannel::google_closure_ptr done) {
        if (m_is_init) {
            return;
        }
        m_controller = controller;
        m_request = request;
        m_response = response;
        m_closure = done;
        m_is_init = true;
    }

    void RPCChannel::updateCache(const std::string &service_name, std::string &server_list) {
        std::vector<std::string> server_list_vec;
        splitStrToVector(server_list, ",", server_list_vec);
        std::set<NetAddr::ptr, CompNetAddr> server_list_set;
        for (const auto &server: server_list_vec) {
            server_list_set.emplace(std::make_shared<IPNetAddr>(server));
        }
        m_service_servers_cache.emplace(service_name, server_list_set);
    }

    void RPCChannel::serviceDiscovery(const std::string &service_name) {
        auto io_thread = std::make_unique<IOThread>();
        auto register_client = std::make_shared<TCPClient>(m_register_center_addr, io_thread->getEventLoop());
        HTTPManager::body_type body;
        body["service_name"] = service_name;
        auto request = std::make_shared<HTTPRequest>();
        auto channel = shared_from_this();
        HTTPManager::createRequest(request, HTTPManager::MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, body);
        register_client->connect([register_client, request, channel, service_name]() {
            register_client->sendRequest(request,
                                         [register_client, request, channel, service_name](HTTPRequest::ptr msg) {
                                             register_client->recvResponse(request->m_msg_id,
                                                                           [register_client, request, channel, service_name](
                                                                                   HTTPResponse::ptr msg) {
                                                                               register_client->getEventLoop()->stop();
                                                                               // 更新本地缓存
                                                                               auto server_list_str = msg->m_response_body_data_map["server_list"];
                                                                               channel->updateCache(service_name,
                                                                                                    server_list_str);
                                                                               INFOLOG("%s | get server cache from register center, server list [%s]",
                                                                                       msg->m_msg_id.c_str(),
                                                                                       channel->getAllServerList().c_str());
                                                                           });
                                         });
        });
        io_thread->start();
    }

    std::string RPCChannel::getAllServerList() {
        std::string tmp = "";
        for (const auto &item: m_service_servers_cache) {
            tmp += "{" + item.first + ":";
            for (const auto &item: item.second) {
                tmp += item->toString() + ",";
            }
            tmp = tmp.substr(0, tmp.size() - 1);
            tmp += "}";
        }
        return tmp;
    }

    void RPCChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                                google::protobuf::Message *response, google::protobuf::Closure *done) {
        // 为空即去执行注册
        if (m_service_servers_cache.empty()) {
            serviceDiscovery(method->service()->full_name());
        }
        // 选择服务器地址
        m_client.reset();
        m_client = std::make_shared<TCPClient>(*m_service_servers_cache["Order"].begin());

        auto request_protocol = std::make_shared<HTTPRequest>();
        auto rpc_controller = dynamic_cast<RPCController *>(controller);
        if (rpc_controller == nullptr) {
            ERRORLOG("failed call method, RpcController convert error");
            return;
        }
        // Order.makeOrder
        auto method_full_name = method->full_name();
        std::string req_pb_data;
        request->SerializeToString(&req_pb_data);

        HTTPManager::body_type body;
        body["method_full_name"] = method_full_name;
        body["pb_data"] = req_pb_data;

        HTTPManager::createRequest(request_protocol, HTTPManager::MSGType::RPC_METHOD_REQUEST, body);

        if (rpc_controller->GetMSGID().empty()) {
            // 外部没有设置controller 的msg id的话用这个覆盖
            rpc_controller->SetMsgId(request_protocol->m_msg_id);
        } else {
            // 否则用外部的controller覆盖默认生成的request msg id
            request_protocol->m_msg_id = rpc_controller->GetMSGID();
        }

        INFOLOG("%s | call method name [%s]", request_protocol->m_msg_id.c_str(), method_full_name.c_str());

        auto this_channel = shared_from_this();
        m_client->connect([this_channel, request_protocol]() {
            // 发送请求
            this_channel->getClient()->sendRequest(request_protocol, [](HTTPRequest::ptr req) {});
            // 接收响应
            this_channel->getClient()->recvResponse(request_protocol->m_msg_id,
                                                    [this_channel, request_protocol](HTTPResponse::ptr res) {
                                                        this_channel->getResponse()->ParseFromString(
                                                                res->m_response_body_data_map["pb_data"]);
                                                        INFOLOG("%s | success get rpc response, rsp_protocol_body [%s], peer addr [%s], local addr[%s], response [%s]",
                                                                res->m_msg_id.c_str(), res->m_response_body.c_str(),
                                                                this_channel->getClient()->getPeerAddr()->toString().c_str(),
                                                                this_channel->getClient()->getLocalAddr()->toString().c_str(),
                                                                this_channel->getResponse()->ShortDebugString().c_str());
                                                        if (this_channel->getClosure()) {
                                                            this_channel->getClosure()->Run();
                                                        }
                                                    });
        });
    }

    google::protobuf::RpcController *RPCChannel::getController() {
        return m_controller.get();
    }

    google::protobuf::Message *RPCChannel::getRequest() {
        return m_request.get();
    }

    google::protobuf::Message *RPCChannel::getResponse() {
        return m_response.get();
    }

    google::protobuf::Closure *RPCChannel::getClosure() {
        return m_closure.get();
    }

    TCPClient *RPCChannel::getClient() {
        return m_client.get();
    }
}
