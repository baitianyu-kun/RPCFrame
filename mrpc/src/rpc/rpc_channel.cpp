//
// Created by baitianyu on 2/11/25.
//
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "rpc/rpc_channel.h"
#include "rpc/rpc_controller.h"
#include "common/log.h"
#include "common/string_util.h"
#include "event/io_thread.h"

namespace mrpc {

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
        auto find = m_service_servers_cache.find(service_name);
        if (find == m_service_servers_cache.end()) {
            auto con_hash = std::make_shared<ConsistentHash>();
            m_service_balance.emplace(service_name, con_hash);
        }
        for (const auto &server: server_list_vec) {
            server_list_set.emplace(std::make_shared<IPNetAddr>(server));
            // 插入到负载均衡中
            m_service_balance[service_name]->addNewPhysicalNode(server, VIRTUAL_NODE_NUM);
        }
        m_service_servers_cache.emplace(service_name, server_list_set);
    }

    std::string RPCChannel::getLocalIP() {
        int sockfd;
        ifconf ifconf;
        ifreq *ifreq = nullptr;
        char buf[512];
        ifconf.ifc_len = 512;
        ifconf.ifc_buf = buf;
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            return std::string{};
        }
        ioctl(sockfd, SIOCGIFCONF, &ifconf); // 设备所有接口
        ifreq = (struct ifreq *) ifconf.ifc_buf;
        for (int i = (ifconf.ifc_len / sizeof(ifreq)); i > 0; i--) {
            if (ifreq->ifr_flags == AF_INET) {
                // 找到ipv4的设备，并且为网卡，需要设置网卡名称
                if (ifreq->ifr_name == std::string(NETWORK_CARD_NAME)) {
                    return std::string(inet_ntoa(((sockaddr_in *) &(ifreq->ifr_addr))->sin_addr));
                }
                ifreq++;
            }
        }
        return std::string{};
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
            register_client->sendRequest(request, [](HTTPRequest::ptr req) {});
            register_client->recvResponse(request->m_msg_id,
                                          [register_client, request, channel, service_name](HTTPResponse::ptr msg) {
                                              register_client->getEventLoop()->stop();
                                              // 更新本地缓存
                                              auto server_list_str = msg->m_response_body_data_map["server_list"];
                                              channel->updateCache(service_name, server_list_str);
                                              INFOLOG("%s | get server cache from register center, server list %s",
                                                      msg->m_msg_id.c_str(), channel->getAllServerList().c_str());
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
        // 为空即去执行服务发现
        if (m_service_servers_cache.empty()) {
            serviceDiscovery(method->service()->full_name());
        }
        // 获取本地ip地址，根据该ip地址去进行hash选择
        auto local_ip = getLocalIP();
        auto server_addr = m_service_balance[method->service()->full_name()]->getServer(local_ip);
        DEBUGLOG("local ip [%s], choosing server [%s]", local_ip.c_str(), server_addr->toString().c_str());
        auto client = std::make_shared<TCPClient>(server_addr);

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
        rpc_controller->SetMsgId(request_protocol->m_msg_id);
        INFOLOG("%s | call method name [%s]", request_protocol->m_msg_id.c_str(), method_full_name.c_str());

        auto this_channel = shared_from_this();
        client->connect([this_channel, request_protocol, client]() {
            // 发送请求
            client->sendRequest(request_protocol, [](HTTPRequest::ptr req) {});
            // 接收响应
            client->recvResponse(request_protocol->m_msg_id,
                                 [this_channel, request_protocol, client](HTTPResponse::ptr res) {
                                     this_channel->getResponse()->ParseFromString(
                                             res->m_response_body_data_map["pb_data"]);
                                     INFOLOG("%s | success get rpc response, peer addr [%s], local addr[%s], response [%s]",
                                             res->m_msg_id.c_str(),
                                             client->getPeerAddr()->toString().c_str(),
                                             client->getLocalAddr()->toString().c_str(),
                                             this_channel->getResponse()->ShortDebugString().c_str());
                                     if (this_channel->getClosure()) {
                                         this_channel->getClosure()->Run();
                                     }
                                     client->getEventLoop()->stop();
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
}
