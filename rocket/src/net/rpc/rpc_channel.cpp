//
// Created by baitianyu on 7/25/24.
//
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "net/rpc/rpc_channel.h"
#include "net/rpc/rpc_controller.h"
#include "net/coder/tinypb/tinypb_protocol.h"
#include "common/msg_id_util.h"
#include "common/string_util.h"
#include "common/log.h"
#include "common/error_code.h"
#include "net/coder/http/http_request.h"
#include "net/coder/http/http_response.h"
#include "coroutine/coroutine_pool.h"

namespace rocket {

    RPCChannel::RPCChannel(NetAddr::net_addr_sptr_t_ register_center_addr,
                           ProtocolType protocol/*ProtocolType::TinyPB_Protocol*/)
            : m_register_center_addr(register_center_addr), m_protocol_type(protocol) {
        m_client = std::make_shared<TCPClient>(m_register_center_addr, m_protocol_type);
    }

    RPCChannel::~RPCChannel() {
        DEBUGLOG("~RPCChannel");
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

        if (m_protocol_type == ProtocolType::HTTP_Protocol) {
            CallMethodHTTPRegisterCenter(method, controller, request, response, done);
            // CallMethodHTTPRegisterCenterCoroutine(method, controller, request, response, done);
            return;
        }

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
        req_protocol->m_method_full_name = method->full_name();
        INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_full_name.c_str());
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

        m_timeout_timer_event_info = std::make_shared<TimerEventInfo>(rpc_controller->GetTimeout(),
                                                                      false, [this_channel, rpc_controller]()mutable {
                    INFOLOG("%s | call rpc timeout", rpc_controller->GetMSGID().c_str());
                    // finished标志结束了调用回调函数
                    if (rpc_controller->Finished()) {
                        this_channel.reset();
                        return;
                    }
                    rpc_controller->StartCancel(); // 取消任务
                    rpc_controller->SetError(ERROR_RPC_CALL_TIMEOUT,
                                             "rpc call timeout " + std::to_string(rpc_controller->GetTimeout()));
                    if (this_channel->GetClosure()) {
                        this_channel->GetClosure()->Run();
                        rpc_controller->SetFinished(true); // 结束调用回调函数
                    }
                    this_channel.reset();
                });
        // 添加到event loop里面
        // rpc channel的event loop负责监听是否连接成功，还有负责超时timer event，一会整理一下各个模块用到的event loop
        this_channel->GetClient()->getEventLoop()->addTimerEvent(m_timeout_timer_event_info);

        // 在client中进行connect，write和read操作
        // 这里如果client的connect返回错误了需要直接return，所以需要在client的connect中来
        // 设置connect的error code来判断是否连接成功，不能只打印一个error
        m_client->connect([req_protocol, this_channel]() mutable {
            // 在这里设置连接失败，并将失败信息放到rpc controller中进行返回
            auto rpc_controller = dynamic_cast<RPCController *>(this_channel->GetController());
            if (this_channel->GetClient()->getConnectErrorCode() != 0) {
                rpc_controller->SetError(this_channel->GetClient()->getConnectErrorCode(),
                                         this_channel->GetClient()->getConnectErrorInfo());
                ERRORLOG("%s | connect error, error code[%d], error info[%s], peer addr[%s]",
                         req_protocol->m_msg_id.c_str(), rpc_controller->GetErrorCode(),
                         rpc_controller->GetErrorInfo().c_str(),
                         this_channel->GetClient()->getPeerAddr()->toString().c_str());
                return;
            }
            // 写入
            this_channel->GetClient()->writeMessage(req_protocol, [req_protocol, this_channel, rpc_controller](
                    AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                INFOLOG("%s | success send rpc request. call method name [%s], peer addr [%s], local addr[%s]",
                        req_protocol->m_msg_id.c_str(), req_protocol->m_method_full_name.c_str(),
                        this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                        this_channel->GetClient()->getLocalAddr()->toString().c_str());
                // 读取
                this_channel->GetClient()->readMessage(req_protocol->m_msg_id, [this_channel, rpc_controller](
                        AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                    auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(msg);
                    // 反序列化失败
                    if (!(this_channel->GetResponse()->ParseFromString(rsp_protocol->m_pb_data))) {
                        ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
                        rpc_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
                        return;
                    }
                    // 协议里面的err code是否为错误
                    if (rsp_protocol->m_err_code != 0) {
                        ERRORLOG("%s | call rpc method [%s] failed, error code [%d], error info [%s]",
                                 rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_full_name.c_str(),
                                 rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());
                        rpc_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
                        return;
                    }
                    INFOLOG("%s | success get rpc response, call method name [%s], peer addr [%s], local addr[%s], response [%s]",
                            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_full_name.c_str(),
                            this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                            this_channel->GetClient()->getLocalAddr()->toString().c_str(),
                            this_channel->GetResponse()->ShortDebugString().c_str());
                    if (this_channel->GetClosure()) {
                        this_channel->GetClosure()->Run();
                    }
                });
            });
        });
    }

    void RPCChannel::CallMethodHTTPRegisterCenterCoroutine(const google::protobuf::MethodDescriptor *method,
                                                           google::protobuf::RpcController *controller,
                                                           const google::protobuf::Message *request,
                                                           google::protobuf::Message *response,
                                                           google::protobuf::Closure *done) {
        auto msg_id = MSGIDUtil::GenerateMSGID();
        std::string final_req_to_register_center = "is_server:" + std::to_string(false) + g_CRLF
                                                   + "method_full_name:" + method->service()->full_name() + g_CRLF
                                                   + "msg_id:" + msg_id;
        rpc_channel_sptr_t_ this_channel = shared_from_this();
        auto req_protocol_register_center = std::make_shared<HTTPRequest>();
        req_protocol_register_center->m_request_body = final_req_to_register_center;
        req_protocol_register_center->m_request_method = HTTPMethod::POST;
        req_protocol_register_center->m_request_version = "HTTP/1.1";
        req_protocol_register_center->m_request_path = "/";
        req_protocol_register_center->m_request_properties.m_map_properties["Content-Length"] = std::to_string(
                final_req_to_register_center.length());
        req_protocol_register_center->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        req_protocol_register_center->m_msg_id = msg_id;

        auto cor = CoroutinePool::GetCoroutinePool()->getCoroutineInstance();
        Coroutine::GetMainCoroutine();

        Coroutine::Resume(cor.get());

        INFOLOG("[MAIN] now begin to connect resume");

        auto fd_event = std::make_shared<FDEvent>(m_client->getClientFD());
        fd_event->setNonBlock();
        fd_event->listen(FDEvent::IN_EVENT, [cor]() {
            Coroutine::Resume(cor.get());
        });
        EventLoop::GetCurrentEventLoop()->addEpollEvent(fd_event);
        int ret = ::connect(m_client->getClientFD(), m_client->getPeerAddr()->getSockAddr(),
                            m_client->getPeerAddr()->getSockAddrLen());
        INFOLOG("[COR %d] now accept begin to yield", cor->getCorId());
        Coroutine::Yield();

        // 然后connect成功后，即epoll监听到了client fd的in event，会跳转到这里来
        // 先删掉事件
        INFOLOG("[COR %d] now connect resume", cor->getCorId());
        fd_event->cancel_listen(FDEvent::IN_EVENT);
        EventLoop::GetCurrentEventLoop()->deleteEpollEvent(fd_event);
        if (ret == 0) {
            INFOLOG("[COR %d] connect success");
        } else {
            INFOLOG("[COR %d] connect failed");
        }
        exit(0);
    }

    void RPCChannel::CallMethodHTTPRegisterCenter(const google::protobuf::MethodDescriptor *method,
                                                  google::protobuf::RpcController *controller,
                                                  const google::protobuf::Message *request,
                                                  google::protobuf::Message *response,
                                                  google::protobuf::Closure *done) {
        // 服务端注册时候注册的是：Order
        // 客户端来的时候是：Order.makeOrder
        // 客户端请求注册中心：is_server, method_full_name，msg_id 返回：success, server_ip, server_port, msg_id
        auto msg_id = MSGIDUtil::GenerateMSGID();
        std::string final_req_to_register_center = "is_server:" + std::to_string(false) + g_CRLF
                                                   + "method_full_name:" + method->service()->full_name() + g_CRLF
                                                   + "msg_id:" + msg_id;
        rpc_channel_sptr_t_ this_channel = shared_from_this();
        auto req_protocol_register_center = std::make_shared<HTTPRequest>();
        req_protocol_register_center->m_request_body = final_req_to_register_center;
        req_protocol_register_center->m_request_method = HTTPMethod::POST;
        req_protocol_register_center->m_request_version = "HTTP/1.1";
        req_protocol_register_center->m_request_path = "/";
        req_protocol_register_center->m_request_properties.m_map_properties["Content-Length"] = std::to_string(
                final_req_to_register_center.length());
        req_protocol_register_center->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        req_protocol_register_center->m_msg_id = msg_id;

        // response_body_map["msg_id"]和req_protocol_register_center->m_msg_id里面的msg id实际上都相等
        // 为了方面每次请求和返回时候都带上msg id
        // dispatcher中处理response时候，response的msg id和request的msg id得相等
        // 连接注册中心
        m_client->connect(
                [req_protocol_register_center, this_channel, method, controller, request, response, done]() mutable {
                    this_channel->GetClient()->writeMessage(req_protocol_register_center,
                                                            [req_protocol_register_center, this_channel](
                                                                    AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                                                                INFOLOG("%s | success send get server info request. call method name [%s], peer addr [%s], local addr[%s]",
                                                                        req_protocol_register_center->m_msg_id.c_str(),
                                                                        req_protocol_register_center->m_request_body.c_str(),
                                                                        this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                                                                        this_channel->GetClient()->getLocalAddr()->toString().c_str());
                                                            });
                    this_channel->GetClient()->readMessage(req_protocol_register_center->m_msg_id,
                                                           [this_channel, method, controller, request, response, done](
                                                                   AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                                                               auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(
                                                                       msg);
                                                               std::unordered_map<std::string, std::string> response_body_map;
                                                               splitStrToMap(rsp_protocol->m_response_body, g_CRLF, ":",
                                                                             response_body_map);
                                                               rsp_protocol->m_msg_id = response_body_map["msg_id"];
                                                               INFOLOG("%s | success get server info, rsp_protocol_body [%s], peer addr [%s],"
                                                                       " local addr[%s], server ip [%s], server port [%s]",
                                                                       rsp_protocol->m_msg_id.c_str(),
                                                                       rsp_protocol->m_response_body.c_str(),
                                                                       this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                                                                       this_channel->GetClient()->getLocalAddr()->toString().c_str(),
                                                                       response_body_map["server_ip"].c_str(),
                                                                       response_body_map["server_port"].c_str());
                                                               this_channel->GetClient()->stop();
                                                               auto server_addr = std::make_shared<IPNetAddr>(
                                                                       response_body_map["server_ip"],
                                                                       std::stoi(response_body_map["server_port"]));
                                                               this_channel->CallMethodHTTP(method, controller, request,
                                                                                            response, done,
                                                                                            server_addr);
                                                           });
                });
    }

    void RPCChannel::CallMethodHTTP(const google::protobuf::MethodDescriptor *method,
                                    google::protobuf::RpcController *controller,
                                    const google::protobuf::Message *request, google::protobuf::Message *response,
                                    google::protobuf::Closure *done,
                                    NetAddr::net_addr_sptr_t_ server_addr) {

        // 根据新的server创建新的client，否则之前的client里面的fd不关闭的话，新的没办法继续使用
        m_client.reset();
        m_client = std::make_shared<TCPClient>(server_addr, m_protocol_type);

        rpc_channel_sptr_t_ this_channel = shared_from_this();
        auto req_protocol = std::make_shared<HTTPRequest>();
        auto rpc_controller = dynamic_cast<RPCController *>(controller);
        if (rpc_controller == nullptr) {
            ERRORLOG("failed call method, RpcController convert error");
            return;
        }
        if (rpc_controller->GetMSGID().empty()) {
            req_protocol->m_msg_id = MSGIDUtil::GenerateMSGID();
            rpc_controller->SetMsgId(req_protocol->m_msg_id);
        } else {
            req_protocol->m_msg_id = rpc_controller->GetMSGID();
        }
        auto method_full_name = method->full_name();
        INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), method_full_name.c_str());
        std::string req_pb_data;
        request->SerializeToString(&req_pb_data);
        std::string final_res = "method_full_name:" + method_full_name + g_CRLF
                                + "pb_data:" + req_pb_data + g_CRLF
                                + "msg_id:" + req_protocol->m_msg_id;

        req_protocol->m_request_body = final_res;
        req_protocol->m_request_method = HTTPMethod::POST;
        req_protocol->m_request_version = "HTTP/1.1";
        req_protocol->m_request_path = "/";
        req_protocol->m_request_properties.m_map_properties["Content-Length"] = std::to_string(final_res.length());
        req_protocol->m_request_properties.m_map_properties["Content-Type"] = content_type_text;

        m_client->connect([req_protocol, this_channel]() mutable {
            // 在这里设置连接失败，并将失败信息放到rpc controller中进行返回
            auto rpc_controller = dynamic_cast<RPCController *>(this_channel->GetController());
            // 写入
            // TODO 加入其他出错的处理，例如超时
            this_channel->GetClient()->writeMessage(req_protocol, [req_protocol, this_channel, rpc_controller](
                    AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                INFOLOG("%s | success send rpc request. call method name [%s], peer addr [%s], local addr[%s]",
                        req_protocol->m_msg_id.c_str(), req_protocol->m_request_body.c_str(),
                        this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                        this_channel->GetClient()->getLocalAddr()->toString().c_str());
            });
            // 读取
            this_channel->GetClient()->readMessage(req_protocol->m_msg_id, [this_channel, rpc_controller](
                    AbstractProtocol::abstract_pro_sptr_t_ msg) mutable {
                auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(msg);

                std::unordered_map<std::string, std::string> response_body_map;
                splitStrToMap(rsp_protocol->m_response_body, g_CRLF, ":", response_body_map);
                this_channel->GetResponse()->ParseFromString(response_body_map["pb_data"]);
                rsp_protocol->m_msg_id = response_body_map["msg_id"];

                INFOLOG("%s | success get rpc response, rsp_protocol_body [%s], peer addr [%s], local addr[%s], response [%s]",
                        rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_response_body.c_str(),
                        this_channel->GetClient()->getPeerAddr()->toString().c_str(),
                        this_channel->GetClient()->getLocalAddr()->toString().c_str(),
                        this_channel->GetResponse()->ShortDebugString().c_str());
                if (this_channel->GetClosure()) {
                    this_channel->GetClosure()->Run();
                }
            });
        });
    }

    // get方法不会增加引用计数，只有复制shared ptr的时候才会增加引用计数
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

    void RPCChannel::error_call_back() {

    }

}



