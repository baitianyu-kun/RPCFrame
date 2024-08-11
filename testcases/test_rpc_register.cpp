//
// Created by baitianyu on 8/11/24.
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
#include "net/register/register_center.h"

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::RegisterCenter registerCenter(addr, rocket::ProtocolType::HTTP_Protocol);
    registerCenter.start();
    return 0;
}