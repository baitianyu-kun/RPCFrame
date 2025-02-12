//
// Created by baitianyu on 2/11/25.
//

#ifndef RPCFRAME_RPC_CHANNEL_H
#define RPCFRAME_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_client.h"
#include "net/balance/hash_balance.h"

#define NEW_MESSAGE(type, var_name) \
        std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEW_RPC_CONTROLLER(var_name) \
        std::shared_ptr<rocket::RPCController> var_name = std::make_shared<rocket::RPCController>(); \

#define NEW_RPC_CHANNEL(addr, var_name) \
        std::shared_ptr<rocket::RPCChannel> var_name = std::make_shared<rocket::RPCChannel>(addr); \

#define CALL_RPC(addr, stub_name, method_name, controller, request, response, closure) \
        {                                                                              \
            channel->init(controller, request, response, closure); \
            stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
        }

// channel连接注册中心进行discovery，注册中心收到后记录下channel的地址，然后向channel推送消息
// channel是tcpclient，也是tcpserver，用来接收注册中心推送的消息
namespace rocket {
    class RPCChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RPCChannel> {
    public:
        using ptr = std::shared_ptr<RPCChannel>;
        using google_rpc_controller_ptr = std::shared_ptr<google::protobuf::RpcController>;
        using google_message_ptr = std::shared_ptr<google::protobuf::Message>;
        using google_closure_ptr = std::shared_ptr<google::protobuf::Closure>;

        explicit RPCChannel(NetAddr::ptr register_center_addr);

        ~RPCChannel() override;

        void init(google_rpc_controller_ptr controller, google_message_ptr request,
                  google_message_ptr response, google_closure_ptr done);

        void serviceDiscovery(const std::string &service_name);

        std::string getAllServerList();

        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done) override;

        google::protobuf::RpcController *getController();

        google::protobuf::Message *getRequest();

        google::protobuf::Message *getResponse();

        google::protobuf::Closure *getClosure();

        TCPClient *getClient();

    private:
        void updateCache(const std::string &service_name, std::string &server_list);

    private:
        TCPClient::ptr m_client{nullptr};
        NetAddr::ptr m_register_center_addr; // 本地注册中心地址，临时将注册中心用作server地址，来测试基本功能
        bool m_is_init{false}; // 是否初始化
        google_rpc_controller_ptr m_controller{nullptr};
        google_message_ptr m_request{nullptr};
        google_message_ptr m_response{nullptr};
        google_closure_ptr m_closure{nullptr};

        std::unordered_map<std::string, std::set<NetAddr::ptr, CompNetAddr>> m_service_servers_cache; // method对应的多少个server
        std::unordered_map<std::string, ConsistentHash::ptr> m_method_balance; // 一个方法对应一个balance
    };
}

#endif //RPCFRAME_RPC_CHANNEL_H
