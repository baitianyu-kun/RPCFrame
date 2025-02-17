//
// Created by baitianyu on 7/20/24.
//
#include "net/tcp/tcp_acceptor.h"
#include "common/log.h"

namespace mrpc {
    TCPAcceptor::TCPAcceptor(NetAddr::ptr local_addr) : m_local_addr(local_addr) {
        // 在IPNetAddr::IPNetAddr中if条件不满足，即ip地址是不合法的情况下，
        // 实际上就相当于构造函数没有修改任何成员变量，例如mip和mport都保持默认值，
        // 即返回一个成员变量都是默认值的对象，并不是不返回对象，所以这里要判断里面是不是默认的对象，如果是默认对象的话
        // 说明创建ip的时候ip不合法，不应该连接
        if (!local_addr->checkValid()) {
            exit(0);
        }
        m_family = m_local_addr->getFamily();
        m_listenfd = socket(m_family, SOCK_STREAM, 0);
        if (m_listenfd < 0) {
            ERRORLOG("invalid listenfd %d", m_listenfd);
            exit(0);
        }

        // 设置socket立即重用，如果处于time wait的连接状态，那么不用等待可以立即重用该socket
        int reuse = 1;
        // level 参数指定要操作哪个协议的选项 (即属性), 指定为SOL_SOCKET
        if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
            ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno));
        }
        if (bind(m_listenfd, m_local_addr->getSockAddr(), m_local_addr->getSockAddrLen()) != 0) {
            ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno));
            exit(0);
        }
        if (listen(m_listenfd, MAX_CONNECTION) != 0) {
            ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno));
            exit(0);
        }
    }

    mrpc::TCPAcceptor::~TCPAcceptor() {
        DEBUGLOG("~TCPAcceptor");
    }

    std::pair<int, NetAddr::ptr> TCPAcceptor::accept() {
        if (m_family == AF_INET) {
            sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr_len);
            // ::accept表示调用全局命名空间中的accept函数，而不是当前命名空间中可能存在的同名函数。
            // 这样做是为了明确使用全局的accept函数，而不会被当前命名空间中的同名函数所隐藏。
            int client_fd = ::accept(m_listenfd, (sockaddr *) (&client_addr), &client_addr_len);
            if (client_fd < 0) {
                ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno));
            }
            auto peer_addr = std::make_shared<IPNetAddr>(client_addr);
            INFOLOG("A client have accpeted succ, client addr [%s]", peer_addr->toString().c_str());
            return std::make_pair(client_fd, peer_addr);
        } else {
            // 其他协议，先设置个默认值
            return std::make_pair(-1, nullptr);
        }
    }

    int TCPAcceptor::getListenFD() {
        return m_listenfd;
    }
}


