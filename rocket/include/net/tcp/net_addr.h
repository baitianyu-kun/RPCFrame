//
// Created by baitianyu on 7/19/24.
//

#ifndef RPCFRAME_NET_ADDR_H
#define RPCFRAME_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket {
    // 封装了大小端转换等操作
    class NetAddr {
    public:
        using net_addr_sptr_t_ = std::shared_ptr<NetAddr>;

        virtual sockaddr *getSockAddr() = 0;

        virtual socklen_t getSockLen() = 0;

        virtual int getFamily() = 0;

        virtual std::string toString() = 0;

        virtual bool checkValid() = 0;
    };

    class IPNetAddr:public NetAddr{
    public:
        
    };
}

#endif //RPCFRAME_NET_ADDR_H
