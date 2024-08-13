//
// Created by baitianyu on 7/25/24.
//

#ifndef RPCFRAME_RPC_CHANNEL_H
#define RPCFRAME_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_client.h"
#include "net/timer_fd_event.h"
#include "net/coder/abstract_protocol.h"

#define NEW_MESSAGE(type, var_name) \
        std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEW_RPC_CONTROLLER(var_name) \
        std::shared_ptr<rocket::RPCController> var_name = std::make_shared<rocket::RPCController>(); \

#define NEW_RPC_CHANNEL(addr, var_name) \
        std::shared_ptr<rocket::RPCChannel> var_name = std::make_shared<rocket::RPCChannel>(addr); \

#define CALL_RPC(addr, stub_name, method_name, controller, request, response, closure) \
        {                                                                              \
            channel->Init(controller, request, response, closure); \
            stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
        }                                                                              \

namespace rocket {

// public std::enable_shared_from_this<RPCChannel>
// 生成智能指针，保证RPCChannel对象一直存在，否则超过作用域后RPCChannel就会析构，那么里面
// 也就无法保证回调函数中google_rpc_controller_sptr_t_和google_message_sptr_t_等存在了，
// 也就无法保证能造回调函数中调用以上内容
// 所以还是需要注意生命周期
    class RPCChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RPCChannel> {
    public:
        using rpc_channel_sptr_t_ = std::shared_ptr<RPCChannel>;
        using google_rpc_controller_sptr_t_ = std::shared_ptr<google::protobuf::RpcController>;
        using google_message_sptr_t_ = std::shared_ptr<google::protobuf::Message>;
        using google_closure_sptr_t_ = std::shared_ptr<google::protobuf::Closure>;

        RPCChannel(NetAddr::net_addr_sptr_t_ register_center_addr,
                   ProtocolType protocol = ProtocolType::TinyPB_Protocol);

        ~RPCChannel() override;

        void Init(google_rpc_controller_sptr_t_ controller, google_message_sptr_t_ request,
                  google_message_sptr_t_ response, google_closure_sptr_t_ done);

        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done) override;

        void CallMethodHTTPRegisterCenter(const google::protobuf::MethodDescriptor *method,
                                          google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                                          google::protobuf::Message *response, google::protobuf::Closure *done);

        void CallMethodHTTP(const google::protobuf::MethodDescriptor *method,
                            google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                            google::protobuf::Message *response, google::protobuf::Closure *done,
                            NetAddr::net_addr_sptr_t_ server_addr);

        google::protobuf::RpcController *GetController();

        google::protobuf::Message *GetRequest();

        google::protobuf::Message *GetResponse();

        google::protobuf::Closure *GetClosure();

        TCPClient *GetClient();

        // 将出错情况都设置为其，在该函数中执行回调函数以及设置controller的状态
        void error_call_back();

    private:
        // NetAddr::net_addr_sptr_t_ m_peer_addr{nullptr}; // 得知道要调用的对方的服务器地址
        NetAddr::net_addr_sptr_t_ m_local_addr{nullptr}; // 本地的地址

        google_rpc_controller_sptr_t_ m_controller{nullptr};
        google_message_sptr_t_ m_request{nullptr};
        google_message_sptr_t_ m_response{nullptr};
        google_closure_sptr_t_ m_closure{nullptr};

        TCPClient::tcp_client_sptr_t_ m_client{nullptr};

        TimerEventInfo::time_event_info_sptr_t_ m_timeout_timer_event_info; // 超时时间定时器

        bool m_is_init{false}; // 是否初始化

        ProtocolType m_protocol_type;
    private:
        NetAddr::net_addr_sptr_t_ m_register_center_addr; // 本地注册中心地址
    };
}

#endif //RPCFRAME_RPC_CHANNEL_H
