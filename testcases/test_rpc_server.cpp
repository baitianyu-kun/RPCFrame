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
#include "../bak/rpc_dispatcher.h.bak"
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_server.h"
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "order.pb.h"
#include "net/coder/http/http_coder.h"
#include "net/coder/http/http_define.h"
#include "net/coder/abstract_coder.h"
#include "net/coder/abstract_protocol.h"


class OrderImpl : public Order {
public:
    OrderImpl() = default;

    ~OrderImpl() override = default;

    void makeOrder(google::protobuf::RpcController *controller,
                   const ::makeOrderRequest *request,
                   ::makeOrderResponse *response,
                   ::google::protobuf::Closure *done) override {
        // APPDEBUGLOG("start sleep 5s");
        // sleep(5);
        // APPDEBUGLOG("stop sleep 5s");
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
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::IPNetAddr::net_addr_sptr_t_ register_addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22225);
    auto tcp_server = std::make_shared<rocket::TCPServer>(addr, register_addr, rocket::ProtocolType::HTTP_Protocol);
    // rocket::TCPServer tcp_server(addr, register_addr, rocket::ProtocolType::HTTP_Protocol);
    // tcp_server.registerService(service);
    // tcp_server.start();
    tcp_server->registerService(service);
    tcp_server->start();
    return 0;
}