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

namespace mrpc {
    // 封装了大小端转换等操作
    class NetAddr {
    public:
        using ptr = std::shared_ptr<NetAddr>;

        virtual sockaddr *getSockAddr() = 0;

        virtual socklen_t getSockAddrLen() = 0;

        virtual int getFamily() = 0;

        virtual std::string toString() = 0;

        virtual std::string getStringIP() = 0;

        virtual std::string getStringPort() = 0;

        virtual bool checkValid() = 0;
    };

    class IPNetAddr : public NetAddr {

    public:
        IPNetAddr(const std::string &ip, uint32_t port);

        explicit IPNetAddr(const std::string &addr);

        explicit IPNetAddr(sockaddr_in addr);

        sockaddr *getSockAddr() override;

        socklen_t getSockAddrLen() override;

        int getFamily() override;

        std::string toString() override;

        std::string getStringIP() override;

        std::string getStringPort() override;

        bool checkValid() override;

    private:
        // 静态里面调不了非静态成员，但是非静态成员可以调用静态成员函数，前者因为是static的，所以和类同时存在，不知道有几个对象
        static bool CheckValid(const std::string &addr);

        static bool CheckValid(const std::string &ip, uint32_t port);

    private:
        std::string m_ip;
        uint32_t m_port{0};
        sockaddr_in m_addr;
    };

    struct CompNetAddr {
        bool operator()(const NetAddr::ptr &left, const NetAddr::ptr &right) const {
            if (left->toString() == right->toString()) {
                return false;
            } else {
                return true;
            }
        }
    };
}

#endif //RPCFRAME_NET_ADDR_H
