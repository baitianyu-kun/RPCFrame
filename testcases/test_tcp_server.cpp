//
// Created by baitianyu on 25-2-10.
//
#include "net/tcp/tcp_server.h"
#include "rpc/servlet/register_center_servlet.h"
#include "rpc/servlet/server_servlet.h"

using namespace rocket;

int main() {
    Config::SetGlobalConfig("../conf/rocket.xml");
    Logger::InitGlobalLogger(0);

    auto s1 = std::make_shared<ServerRegisterServlet>();
    auto s2 = std::make_shared<ClientRegisterServlet>();
    auto s3 = std::make_shared<ClientServerServlet>();
    auto s4 = std::make_shared<RegisterUpdateServer>();

    IPNetAddr::ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    auto tcp_server = std::make_shared<rocket::TCPServer>(addr);

    tcp_server->addServlet(RPC_SERVER_REGISTER_PATH, s1);
    tcp_server->addServlet(RPC_CLIENT_REGISTER_DISCOVERY_PATH, s2);
    tcp_server->addServlet(RPC_METHOD_PATH, s3);
    tcp_server->addServlet(RPC_REGISTER_UPDATE_SERVER_PATH, s4);

    tcp_server->start();
    return 0;
}