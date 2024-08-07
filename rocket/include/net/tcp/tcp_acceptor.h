//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_TCP_ACCEPTOR_H
#define RPCFRAME_TCP_ACCEPTOR_H

#include <memory>
#include "net/tcp/net_addr.h"

#define MAX_CONNECTION 1000

namespace rocket {
    class TCPAcceptor {
    public:
        using tcp_acceptor_sptr_t_ = std::shared_ptr<TCPAcceptor>;

        // 使用父类，然后虚函数多态
        explicit TCPAcceptor(NetAddr::net_addr_sptr_t_ local_addr);

        ~TCPAcceptor();

        // fd以及其对应的地址
        std::pair<int, NetAddr::net_addr_sptr_t_> accept();

        int getListenFD();

    private:
        NetAddr::net_addr_sptr_t_ m_local_addr; // 服务端监听的地址
        int m_listenfd{-1}; // 监听套接字
        int m_family{-1};
    };
}

#endif //RPCFRAME_TCP_ACCEPTOR_H
