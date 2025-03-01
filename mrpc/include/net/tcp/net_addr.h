//
// Created by baitianyu on 7/19/24.
//

#ifndef RPCFRAME_NET_ADDR_H
#define RPCFRAME_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
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

    struct CompNetAddr {
        bool operator()(const NetAddr::ptr &left, const NetAddr::ptr &right) const {
            if (left->toString() == right->toString()) {
                return false;
            } else {
                return true;
            }
        }
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

    class UnixDomainSocketAddr : public NetAddr {
    public:
        using ptr = std::shared_ptr<UnixDomainSocketAddr>;

        explicit UnixDomainSocketAddr(const std::string &path); // 构造函数，接受一个文件系统路径作为参数

        explicit UnixDomainSocketAddr(sockaddr_un addr);

        sockaddr *getSockAddr() override; // 返回 sockaddr 结构体指针

        socklen_t getSockAddrLen() override; // 返回 sockaddr 结构体的长度

        int getFamily() override; // 返回地址族（对于 Unix Domain Socket 是 AF_UNIX）

        std::string toString() override; // 返回地址的字符串表示形式

        std::string getStringIP() override; // Unix Domain Socket 没有 IP 地址，返回空字符串

        std::string getStringPort() override; // Unix Domain Socket 没有端口号，返回空字符串

        bool checkValid() override;

    private:
        std::string m_path;
        sockaddr_un m_addr;  // Unix Domain Socket 地址结构体
    };
}

#endif //RPCFRAME_NET_ADDR_H
