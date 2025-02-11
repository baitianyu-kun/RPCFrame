//
// Created by baitianyu on 2/10/25.
//

#ifndef RPCFRAME_RPC_SERVER_H
#define RPCFRAME_RPC_SERVER_H

#include "net/tcp/tcp_server.h"
#include "net/tcp/tcp_client.h"
#include "rpc/servlet/server_servlet.h"

namespace rocket {
    class RPCServer : public TCPServer {
    public:
        RPCServer(NetAddr::ptr local_addr, NetAddr::ptr register_addr);

        ~RPCServer();

        void initServlet();

        void addService(ClientServerServlet::protobuf_service_ptr service);

        void registerToCenter();

        void startRPC();

    private:

        NetAddr::ptr m_local_addr; // 本地监听地址
        NetAddr::ptr m_register_addr; // 注册中心地址

        ClientServerServlet::ptr m_client_server_servlet;

        RegisterUpdateServer::ptr m_register_update_server_servlet;
    };
}

#endif //RPCFRAME_RPC_SERVER_H
