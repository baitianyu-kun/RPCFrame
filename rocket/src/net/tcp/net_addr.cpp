//
// Created by baitianyu on 7/19/24.
//
#include <iostream>
#include "net/tcp/net_addr.h"
#include "common/log.h"
#include "common/util.h"

namespace rocket {

    bool IPNetAddr::CheckValid(const std::string &ip, uint32_t port) {
        if (ip.empty()) {
            ERRORLOG("invalid ipv4 ip %s, port %d", ip.c_str(), port);
            return false;
        }
        if (port < 0 || port > 65536) {
            ERRORLOG("invalid ipv4 ip %s, port %d", ip.c_str(), port);
            return false;
        }
        // 例如，如果“a.b.c.d”地址的一部分超过 255，则inet_addr返回值INADDR_NONE。
        if (inet_addr(ip.c_str()) == INADDR_NONE) {
            ERRORLOG("invalid ipv4 ip %s, port %d", ip.c_str(), port);
            return false;
        }
        return true;
    }

    bool rocket::IPNetAddr::CheckValid(const std::string &addr) {
        size_t idx = addr.find_first_of(":");
        if (idx == addr.npos) {
            ERRORLOG("invalid ipv4 addr %s", addr.c_str());
            return false;
        }
        auto ip = addr.substr(0, idx);
        auto port = addr.substr(idx + 1, addr.size() - idx - 1);
        return CheckValid(ip, atoi(port.c_str()));
    }

    // else的时候实际上就相当于构造函数没有修改任何成员变量，例如mip和mport都保持默认值，即返回一个成员变量都是默认值的对象，并不是不返回对象
    IPNetAddr::IPNetAddr(const std::string &ip, uint32_t port) {
        if (CheckValid(ip, port)) {
            m_ip = ip;
            m_port = port;
            memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin_family = AF_INET;
            // 用于将IPv4地址字符串转换为网络字节序的二进制形式
            // 或者使用inet_pton(AF_INET, ip, &base_address.sin_addr);
            m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
            m_addr.sin_port = htons(m_port);
        }
    }

    IPNetAddr::IPNetAddr(const std::string &addr) {
        if (CheckValid(addr)) {
            size_t idx = addr.find_first_of(":");
            m_ip = addr.substr(0, idx);
            m_port = atoi(addr.substr(idx + 1, addr.size() - idx - 1).c_str());
            // 从一个构造函数里面调用另一个构造函数好像从cpp11才开始支持，委托构造函数
            memset(&m_addr, 0, sizeof(m_addr));
            m_addr.sin_family = AF_INET;
            m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
            m_addr.sin_port = htons(m_port);
        }
    }

    IPNetAddr::IPNetAddr(sockaddr_in addr) : m_addr(addr) {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr)); // 网字节序转字符串
        m_port = ntohs(m_addr.sin_port); // net to short转port为short类型
    }

    sockaddr *IPNetAddr::getSockAddr() {
        return (sockaddr *) &m_addr;
    }

    socklen_t IPNetAddr::getSockAddrLen() {
        return sizeof(m_addr);
    }

    int IPNetAddr::getFamily() {
        return AF_INET;
    }

    std::string IPNetAddr::toString() {
        return m_ip + ":" + std::to_string(m_port);
    }

    bool IPNetAddr::checkValid() {
        return CheckValid(m_ip, m_port);
    }

    std::string IPNetAddr::getStringIP() {
        return m_ip;
    }

    std::string IPNetAddr::getStringPort() {
        return std::to_string(m_port);
    }

}


