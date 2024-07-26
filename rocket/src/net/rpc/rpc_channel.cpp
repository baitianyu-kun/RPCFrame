//
// Created by baitianyu on 7/25/24.
//
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "net/rpc/rpc_channel.h"
#include "net/rpc/rpc_controller.h"
#include "net/coder/tinypb_protocol.h"
#include "common/msg_id_util.h"
#include "common/log.h"
#include "common/error_code.h"


namespace rocket {

    RPCChannel::RPCChannel(NetAddr::net_addr_sptr_t_ peer_addr) : m_peer_addr(peer_addr) {
        m_client = std::make_shared<TCPClient>(m_peer_addr);
    }

    RPCChannel::~RPCChannel() {

    }

    void RPCChannel::Init(RPCChannel::google_rpc_controller_sptr_t_ controller,
                          RPCChannel::google_message_sptr_t_ request,
                          RPCChannel::google_message_sptr_t_ response, RPCChannel::google_closure_sptr_t_ done) {
        if (m_is_init) {
            return;
        }
        m_controller = controller;
        m_request = request;
        m_response = response;
        m_closure = done;
        m_is_init = true;
    }

    // 这里的CallMethod和Service里面的CallMethod(在dispatcher中)基本上差不多
    void rocket::RPCChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                        google::protobuf::RpcController *controller,
                                        const google::protobuf::Message *request, google::protobuf::Message *response,
                                        google::protobuf::Closure *done) {
        // 保存一下当前channel的智能指针，保证回调函数调用时候channel存在，也就间接保证了回调函数调用
        // 时候可以调用google_rpc_controller_sptr_t_和google_message_sptr_t_等内容
        rpc_channel_sptr_t_ this_channel = shared_from_this();
        // 创建request并把request message放里面的pb data中
        auto req_protocol = std::make_shared<TinyPBProtocol>();
        auto rpc_controller = dynamic_cast<RPCController *>(controller);
        if (rpc_controller == nullptr) {
            ERRORLOG("failed callmethod, RpcController convert error");
            return;
        }
        if (rpc_controller->GetMSGID().empty()) {
            req_protocol->m_msg_id = MSGIDUtil::GenerateMSGID();
            rpc_controller->SetMsgId(req_protocol->m_msg_id);
        } else {
            req_protocol->m_msg_id = rpc_controller->GetMSGID();
        }
        req_protocol->m_method_name = method->full_name();
        INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());
        // 判断是否init
        if (!m_is_init) {
            std::string err_info = "RpcChannel not call init()";
            rpc_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
            ERRORLOG("%s | %s, RpcChannel not init ", req_protocol->m_msg_id.c_str(), err_info.c_str());
            return;
        }
        // request的序列化，把request message序列化放到pb data里面
        if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
            std::string err_info = "failed to serialize";
            rpc_controller->SetError(ERROR_FAILED_SERIALIZE, err_info); // 设置controller的error
            ERRORLOG("%s | %s, origin request [%s] ", req_protocol->m_msg_id.c_str(), err_info.c_str(),
                     request->ShortDebugString().c_str());
            return;
        }

        // 在client中进行connect，write和read操作
        m_client->connect([req_protocol, this_channel]() {
            // 写入
            this_channel->GetClient()->writeMessage(req_protocol, [req_protocol, this_channel](
                    AbstractProtocol::abstract_pro_sptr_t_ msg) {
                INFOLOG("%s | send rpc request success. call method name [%s], peer addr [%s]",
                        req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str(),
                        this_channel->GetClient()->getPeerAddr()->toString().c_str());
                // 读取
                this_channel->GetClient()->readMessage(req_protocol->m_msg_id, [this_channel](
                        AbstractProtocol::abstract_pro_sptr_t_ msg) {

                    auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg);
                    INFOLOG("%s | success get rpc response, call method name [%s], peer addr [%s]",
                            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
                            this_channel->GetClient()->getPeerAddr()->toString().c_str());

                    auto rpc_controller = dynamic_cast<RPCController *>(this_channel->GetController());
                    if (!(this_channel->GetResponse()->ParseFromString(rsp_protocol->m_pb_data))) {
                        ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
                        rpc_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
                        return;
                    }
                    if (rsp_protocol->m_err_code != 0) {
                        ERRORLOG("%s | call rpc method [%s] failed, error code [%d], error info [%s]",
                                 rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
                                 rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());
                        rpc_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
                        return;
                    }

                    INFOLOG("%s | return rpc success, call method name [%s], peer addr [%s], response [%s]",
                            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
                            this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                            this_channel->GetResponse()->ShortDebugString().c_str());

                    if (this_channel->GetClosure()) {
                        this_channel->GetClosure()->Run();
                    }
                    // 释放this channel的指针并设置为nullptr，上面需要添加mutable才可以进行修改，因为lambda默认不允许修改其捕获的外部变量
                    // this_channel.reset();
                });
            });
        });
    }

    google::protobuf::RpcController *RPCChannel::GetController() {
        return m_controller.get();
    }

    google::protobuf::Message *RPCChannel::GetRequest() {
        return m_request.get();
    }

    google::protobuf::Message *RPCChannel::GetResponse() {
        return m_response.get();
    }

    google::protobuf::Closure *RPCChannel::GetClosure() {
        return m_closure.get();
    }

    TCPClient *RPCChannel::GetClient() {
        return m_client.get();
    }
}



