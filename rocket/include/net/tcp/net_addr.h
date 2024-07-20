//
// Created by baitianyu on 7/19/24.
//

#ifndef RPCFRAME_NET_ADDR_H
#define RPCFRAME_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <memory>

namespace rocket {
    // 封装了大小端转换等操作
    class NetAddr {
    public:
        using net_addr_sptr_t_ = std::shared_ptr<NetAddr>;

        virtual sockaddr *getSockAddr() = 0;

        virtual socklen_t getSockAddrLen() = 0;

        virtual int getFamily() = 0;

        virtual std::string toString() = 0;

        virtual bool checkValid() = 0;
    };

    class IPNetAddr : public NetAddr {
    public:
        // 静态里面调不了非静态成员，但是非静态成员可以调用静态成员函数，前者因为是static的，所以和类同时存在，不知道有几个对象
        static bool CheckValid(const std::string &addr);

        static bool CheckValid(const std::string &ip, uint32_t port);

    public:
        // uint16_t port会出现到65536的时候溢出到0，很傻逼的设计，应该用32的
        IPNetAddr(const std::string &ip, uint32_t port);

        explicit IPNetAddr(const std::string &addr);

        explicit IPNetAddr(sockaddr_in addr);

        sockaddr *getSockAddr() override;

        socklen_t getSockAddrLen() override;

        int getFamily() override;

        std::string toString() override;

        bool checkValid() override;

    private:
        std::string m_ip;
        uint32_t m_port{0};
        sockaddr_in m_addr;
    };
}

#endif //RPCFRAME_NET_ADDR_H
