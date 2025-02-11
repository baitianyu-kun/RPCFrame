//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_REGISTER_CENTER_H
#define RPCFRAME_REGISTER_CENTER_H

#include <unordered_set>
#include "net/tcp/tcp_server.h"
#include "net/tcp/tcp_client.h"

namespace rocket {

    class RegisterCenter : public TCPServer {

    private:
        NetAddr::ptr m_local_addr;

    public:
        using ptr = std::unique_ptr<RegisterCenter>;

        RegisterCenter(NetAddr::ptr local_addr);

        ~RegisterCenter();

        void initServlet();

        void startRegisterCenter();

    private:
        // Order.makeOrder, 提供Order的所有服务器，{Order:{Server1, Server2...}}
        // 全部返回给客户端，客户端使用负载均衡进行选择
        // Service -> IPS
        std::unordered_map<std::string, std::unordered_set<NetAddr::ptr, CompNetAddr>> m_service_servers;
        // IP -> Service，每个ip对应的服务，遍历这个集合检测IP是否有效，无效的话获取该IP对应的Service，然后m_service_servers[Service].erase(IP)
        std::unordered_map<NetAddr::ptr, std::string> m_servers_service;
    public:
        // 添加的话需要进行加锁
        void handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

        void handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

    public:
        void notifyClientServerFailed(); // 主动通知客户端服务器失效

    private:
        void publishClientMessage();
    };

}

#endif //RPCFRAME_REGISTER_CENTER_H
