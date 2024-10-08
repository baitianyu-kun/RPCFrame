//
// Created by baitianyu on 7/25/24.
//
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "common/log.h"
#include "common/config.h"
#include "common/log.h"
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_client.h"
#include "net/coder/string_coder.h"
#include "net/coder/abstract_protocol.h"
#include "net/coder/tinypb/tinypb_protocol.h"
#include "net/coder/tinypb/tinypb_coder.h"
#include <google/protobuf/service.h>
#include "order.pb.h"
#include "net/rpc/rpc_channel.h"
#include "net/rpc/rpc_controller.h"
#include "net/rpc/rpc_closure.h"
#include "thread"


void test_rpc_channel_timeout_marcos_http() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::IPNetAddr::net_addr_sptr_t_ register_addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22225);
    auto channel = std::make_shared<rocket::RPCChannel>(register_addr, rocket::ProtocolType::HTTP_Protocol);
    auto request = std::make_shared<makeOrderRequest>();
    request->set_price(100);
    request->set_goods("apple");
    auto response = std::make_shared<makeOrderResponse>();
    auto controller = std::make_shared<rocket::RPCController>();
    // controller->SetMsgId("99998888");
    auto closure = std::make_shared<rocket::RPCClosure>([request, response, channel, controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]",
                    request->ShortDebugString().c_str(),
                    response->ShortDebugString().c_str());
            // 执行业务逻辑
            if (response->order_id() == "20230514") {
                INFOLOG("hello");
            }
        } else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
                     request->ShortDebugString().c_str(),
                     controller->GetErrorCode(),
                     controller->GetErrorInfo().c_str());
        }
        INFOLOG("now exit client event loop");
        channel->GetClient()->stop();
        channel.reset();
    });
    controller->SetTimeout(2000); // 设置超时时间
    channel->Init(controller, request, response, closure);
    Order_Stub stub(channel.get());
    stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
}

// void test_rpc_channel_timeout_marcos() {
//     rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
//     NEW_RPC_CHANNEL(addr, channel);
//     NEW_MESSAGE(makeOrderRequest, request);
//     NEW_MESSAGE(makeOrderResponse, response);
//     request->set_price(100);
//     request->set_goods("apple");
//     NEW_RPC_CONTROLLER(controller);
//     controller->SetMsgId("99998888");
//     controller->SetTimeout(6000);
//     auto closure = std::make_shared<rocket::RPCClosure>([request, response, channel, controller]() mutable {
//         if (controller->GetErrorCode() == 0) {
//             INFOLOG("call rpc success, request[%s], response[%s]",
//                     request->ShortDebugString().c_str(),
//                     response->ShortDebugString().c_str());
//             // 执行业务逻辑
//             if (response->order_id() == "20230514") {
//                 INFOLOG("hello");
//             }
//         } else {
//             ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
//                      request->ShortDebugString().c_str(),
//                      controller->GetErrorCode(),
//                      controller->GetErrorInfo().c_str());
//         }
//         INFOLOG("now exit client event loop");
//         channel->GetClient()->stop();
//         channel.reset();
//     });
//     CALL_RPC(addr, Order_Stub, makeOrder, controller, request, response, closure);
// }
//
// void test_rpc_channel_timeout() {
//     rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
//     auto channel = std::make_shared<rocket::RPCChannel>(addr);
//     auto request = std::make_shared<makeOrderRequest>();
//     request->set_price(100);
//     request->set_goods("apple");
//     auto response = std::make_shared<makeOrderResponse>();
//     auto controller = std::make_shared<rocket::RPCController>();
//     controller->SetMsgId("99998888");
//     auto closure = std::make_shared<rocket::RPCClosure>([request, response, channel, controller]() mutable {
//         if (controller->GetErrorCode() == 0) {
//             INFOLOG("call rpc success, request[%s], response[%s]",
//                     request->ShortDebugString().c_str(),
//                     response->ShortDebugString().c_str());
//             // 执行业务逻辑
//             if (response->order_id() == "20230514") {
//                 INFOLOG("hello");
//             }
//         } else {
//             ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
//                      request->ShortDebugString().c_str(),
//                      controller->GetErrorCode(),
//                      controller->GetErrorInfo().c_str());
//         }
//         INFOLOG("now exit client event loop");
//         channel->GetClient()->stop();
//         channel.reset();
//     });
//     controller->SetTimeout(2000); // 设置超时时间
//     channel->Init(controller, request, response, closure);
//     Order_Stub stub(channel.get());
//     stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
// }
//
// void test_rpc_channel() {
//     rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
//     auto channel = std::make_shared<rocket::RPCChannel>(addr);
//     auto request = std::make_shared<makeOrderRequest>();
//     request->set_price(100);
//     request->set_goods("apple");
//     auto response = std::make_shared<makeOrderResponse>();
//     auto controller = std::make_shared<rocket::RPCController>();
//     controller->SetMsgId("99998888");
//     auto closure = std::make_shared<rocket::RPCClosure>([request, response, channel]() mutable {
//         INFOLOG("client call rpc success, request [%s], response [%s]", request->ShortDebugString().c_str(),
//                 response->ShortDebugString().c_str());
//         INFOLOG("now exit client event loop");
//         channel->GetClient()->stop();
//         channel.reset();
//     });
//     channel->Init(controller, request, response, closure);
//     Order_Stub stub(channel.get());
//     stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
// }
//
// void test_rpc_client() {
//     rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
//     rocket::TCPClient client(addr);
//     client.connect([addr, &client]() {
//         DEBUGLOG("connect to [%s] success", addr->toString().c_str());
//         std::shared_ptr<rocket::TinyPBProtocol> message = std::make_shared<rocket::TinyPBProtocol>();
//         message->m_msg_id = "99998888";
//         makeOrderRequest request;
//         request.set_price(100);
//         request.set_goods("apple");
//         if (!request.SerializeToString(&(message->m_pb_data))) {
//             ERRORLOG("serialize error");
//             return;
//         }
//         message->m_method_full_name = "Order.makeOrder";
//         client.writeMessage(message, [request](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
//             DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
//         });
//         client.readMessage("99998888", [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
//             std::shared_ptr<rocket::TinyPBProtocol> message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(
//                     msg_ptr);
//             DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
//             makeOrderResponse response;
//             if (!response.ParseFromString(message->m_pb_data)) {
//                 ERRORLOG("deserialize error");
//                 return;
//             }
//             DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
//         });
//     });
// }

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0, false);
    // test_rpc_channel_timeout_marcos();

    test_rpc_channel_timeout_marcos_http();
    // for (int i = 0; i < 2000; i++) {
    //     std::thread t1(test_rpc_channel_timeout_marcos_http);
    //     t1.join();
    // }
}

