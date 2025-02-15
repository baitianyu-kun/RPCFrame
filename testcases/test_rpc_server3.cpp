//
// Created by baitianyu on 2/11/25.
//
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "rpc/rpc_server.h"
#include "order.pb.h"

using namespace mrpc;

class OrderImpl : public Order {
public:
    OrderImpl() = default;

    ~OrderImpl() override = default;

    void makeOrder(google::protobuf::RpcController *controller,
                   const ::makeOrderRequest *request,
                   ::makeOrderResponse *response,
                   ::google::protobuf::Closure *done) override {
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20230514");
        if (done) {
            done->Run();
            delete done;
            done = nullptr;
        }
    }
};

int main() {
    Config::SetGlobalConfig("../conf/mrpc.xml");
    Logger::InitGlobalLogger(0);

    auto local_addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 22229);
    auto register_addr = std::make_shared<mrpc::IPNetAddr>("127.0.0.1", 22225);
    auto rpc_server = std::make_unique<RPCServer>(local_addr, register_addr);

    auto service = std::make_shared<OrderImpl>();
    rpc_server->addService(service);

    rpc_server->startRPC();
    return 0;
}