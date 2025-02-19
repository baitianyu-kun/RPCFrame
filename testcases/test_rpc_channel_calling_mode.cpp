//
// Created by baitianyu on 2/11/25.
//
#include <unistd.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "rpc/rpc_server.h"
#include "rpc/rpc_channel.h"
#include "order.pb.h"

using namespace mrpc;

int main() {
    Config::SetGlobalConfig("../conf/mrpc.xml");
    Logger::InitGlobalLogger(0);

    auto channel = std::make_shared<RPCChannel>();

    auto request_msg = std::make_shared<makeOrderRequest>();
    request_msg->set_price(100);
    request_msg->set_goods("apple");

    Order_Stub stub(channel.get());

    channel->callRPCAsync<makeOrderRequest, makeOrderResponse>(
            std::bind(&Order_Stub::makeOrder, &stub, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3, std::placeholders::_4),
            request_msg,
            [](std::shared_ptr<makeOrderResponse> response_msg) {
                if (response_msg->order_id() == "20230514") {
                    INFOLOG("========= Success Call RPC By Async ==============");
                }
            }
    );

    auto future = channel->callRPCFuture<makeOrderRequest, makeOrderResponse>(
            std::bind(&Order_Stub::makeOrder, &stub, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3, std::placeholders::_4),
            request_msg);
    auto response_msg = future.get();
    if (response_msg->order_id() == "20230514") {
        INFOLOG("========= Success Call RPC By Future ==============");
    }
    return 0;
}