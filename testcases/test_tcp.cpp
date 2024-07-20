//
// Created by baitianyu on 7/20/24.
//
#include "../include/net/tcp/net_addr.h"
#include "../include/common/log.h"
#include "../include/net/tcp/tcp_server.h"

void test_tcp_server(){
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    DEBUGLOG("create addr %s", addr->toString().c_str());
    rocket::TCPServer tcpServer(addr);
    tcpServer.start();
}

void test_tcp_create() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.144", 22224);
    DEBUGLOG("create addr %s", addr->toString().c_str());
}

int main(){
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
//    test_tcp_create();
    test_tcp_server();
}