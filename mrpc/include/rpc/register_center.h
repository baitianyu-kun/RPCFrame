//
// Created by baitianyu on 25-2-10.
//

#ifndef RPCFRAME_REGISTER_CENTER_H
#define RPCFRAME_REGISTER_CENTER_H

#include <unordered_set>
#include "net/tcp/tcp_server.h"
#include "net/tcp/tcp_client.h"
#include "common/timestamp.h"

//#define HEART_TIMER_EVENT_INTERVAL 10000

namespace mrpc {

    class RegisterCenter : public TCPServer {

    private:
        NetAddr::ptr m_local_addr;

    public:
        using ptr = std::unique_ptr<RegisterCenter>;

        explicit RegisterCenter(NetAddr::ptr local_addr);

        ~RegisterCenter();

        void initServlet();

        void startRegisterCenter();

    private:
        // Order.makeOrder, 提供Order的所有服务器，{Order:{Server1, Server2...}}
        // 全部返回给客户端，客户端使用负载均衡进行选择
        // Service -> IPS
        std::unordered_map<std::string, std::set<NetAddr::ptr, CompNetAddr>> m_service_servers;
        // IP -> Service，每个ip对应的服务，遍历这个集合检测IP是否有效，无效的话获取该IP对应的Service，然后m_service_servers[Service].erase(IP)
        // 一个IP可能提供多个服务
        std::unordered_map<std::string, std::vector<std::string>> m_servers_service;
        // 一个IP维护一个定时器，收到心跳包后重新设置计时器
        std::unordered_map<std::string, TimerId> m_servers_timer_event;

        RWMutex m_mutex;

        // 维护一个客户端列表，用来做服务推送，key：订阅的服务名，value：哪些客户端订阅了
        std::unordered_map<std::string, std::set<NetAddr::ptr, CompNetAddr>> m_service_clients;

        void updateServiceServer(std::vector<std::string> all_services_names_vec,
                                 NetAddr::ptr server_addr);

        void serverTimeOut(NetAddr::ptr server_addr);

        TimerEventInfo::ptr m_test_timer_event;

    public:
        std::string getAllServiceNamesStr();

        void handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

        void handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

        void handleClientSubscribe(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

        void handleServerHeart(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

    public:
        void notifyClientServerFailed(); // 主动通知客户端服务器失效

        void publishClientMessage();
    };

}

#endif //RPCFRAME_REGISTER_CENTER_H
