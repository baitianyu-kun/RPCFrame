//
// Created by baitianyu on 7/25/24.
//

#ifndef RPCFRAME_RPC_CHANNEL_H
#define RPCFRAME_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_client.h"

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

        explicit RPCChannel(NetAddr::net_addr_sptr_t_ peer_addr);

        ~RPCChannel() override;

        void Init(google_rpc_controller_sptr_t_ controller, google_message_sptr_t_ request,
                  google_message_sptr_t_ response, google_closure_sptr_t_ done);

        void CallMethod(const google::protobuf::MethodDescriptor *method,
                        google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                        google::protobuf::Message *response, google::protobuf::Closure *done) override;

        // 在这段代码中，m_controller 是一个 shared_ptr 类型的对象，
        // 而函数 RPCChannel::GetController() 返回的是一个 shared_ptr 对象，会导致 m_controller 的引用计数增加。
        // google_rpc_controller_sptr_t_ GetController();
        // 所以这里就不如用裸指针，用get方法

        // 至于出现shared ptr析构以后回调函数仍然有可能调用get方法的话，下面有做分析，在这个项目中
        // 是不会出现的，因为每次回调都要捕获channel的shared ptr，channel在，那么controller
        // 等智能指针以及其对应的get方法获取到的裸指针都存在，所以可以放心使用裸指针。
        google::protobuf::RpcController *GetController();

        google::protobuf::Message *GetRequest();

        google::protobuf::Message *GetResponse();

        google::protobuf::Closure *GetClosure();

        TCPClient *GetClient();

    private:
        NetAddr::net_addr_sptr_t_ m_peer_addr{nullptr}; // 得知道要调用的对方的服务器地址
        NetAddr::net_addr_sptr_t_ m_local_addr{nullptr}; // 本地的地址

        // 保存外面传进来的智能指针。因为call method里面如果写回调函数来调其那几个参数，例如*done啊，则done可能在还没有
        // 调用之前其指向的对象就已经析构了。需要保证在回调函数调用完成后再析构。所以可以先保存一下这些智能指针，只要其计数不归零，那么就
        // 会一直存在。
        // 像void TCPConnection::pushSendMessage里面先放到m_write_dones里面保存一下

        // 上面获取的时候就Get，避免shared ptr引用+1，然后由这里管理生命周期
        // 会不会这里引用计数归零，但是还是使用了get方法的情况。在这里应该是不会有，因为回调函数每次都要捕获
        // channel的shared ptr，只要channel在的话，这些智能指针和其对应的get方法都会存在
        // 但是如果放到别的项目里面，就可能会出现在回调函数的时候channel已经失效了，那么这个时候就需要返回一个
        // 智能指针的值，否则就会出现shared ptr已经析构，但是还是使用了get方法的情况。

        // 由于这里在回调中每次都捕获channel的shared ptr，shared ptr存在的话，那么其他智能指针以及裸指针
        // 一定存在，所以上面可以返回*GetResponse等裸指针。
        google_rpc_controller_sptr_t_ m_controller{nullptr};
        google_message_sptr_t_ m_request{nullptr};
        google_message_sptr_t_ m_response{nullptr};
        google_closure_sptr_t_ m_closure{nullptr};

        TCPClient::tcp_client_sptr_t_ m_client{nullptr};

        bool m_is_init{false}; // 是否初始化

    };

}

#endif //RPCFRAME_RPC_CHANNEL_H
