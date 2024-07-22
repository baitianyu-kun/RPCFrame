//
// Created by baitianyu on 7/22/24.
//
#include <unistd.h>
#include "net/tcp/tcp_client.h"

namespace rocket {
    rocket::TCPClient::TCPClient(NetAddr::net_addr_sptr_t_ peer_addr) : m_peer_addr(peer_addr) {
        m_event_loop = std::move(std::unique_ptr<EventLoop>(EventLoop::GetCurrentEventLoop()));
        m_client_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
        if (m_client_fd < 0) {
            ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
            return;
        }
        m_fd_event = FDEventPool::GetFDEventPool()->getFDEvent(m_client_fd);
        m_fd_event->setNonBlock();
        // 作为client的情况没有本地监听地址local addr
        m_connection = std::make_shared<TCPConnection>(
                m_event_loop,
                m_client_fd,
                MAX_TCP_BUFFER_SIZE,
                peer_addr,
                nullptr,
                TCPConnectionByClient
        );
    }

    TCPClient::~TCPClient() {
        // 需要把刚刚主动连接的socket给销毁掉
        if (m_client_fd > 0) {
            close(m_client_fd);
        }
    }

    void TCPClient::connect(std::function<void()> done) {
        int ret = ::connect(m_client_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockAddrLen());
        if (ret == 0) {
            DEBUGLOG("connect [%s] sussess", m_peer_addr->toString().c_str());
            m_connection->setState(Connected);
            if (done) {
                done();
            }
        } else if (ret == -1) {
            // epoll监听可写事件，就是证明已经连接成功了，此时缓冲区可写，为什么不是可读事件呢？因为此时没有可读的
            // 正在建立连接，可以在epoll中继续建立连接，并判断继续建立的这个连接的错误码做出相应的回复
            if (errno == EINPROGRESS) {
                m_fd_event->listen(FDEvent::OUT_EVENT, [this, done]() {
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_client_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                    if (error == 0) {
                        DEBUGLOG("connect [%s] sussess", m_peer_addr->toString().c_str());
                        if (done) {
                            done();
                        }
                    } else {
                        ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
                    }
                    // 处理完后需要取消监听写事件，否则会一直触发
                    m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                });
                m_event_loop->addEpollEvent(m_fd_event);
                // 添加完事件后记得开启loop循环
                if (!m_event_loop->LoopStopFlag()) {
                    m_event_loop->loop();
                }
            } else {
                ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
            }
        }
    }

    void TCPClient::writeMessage(AbstractProtocol::abstract_pro_sptr_t_ message,
                                 std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> done) {

    }

    void TCPClient::readMessage() {

    }

    void TCPClient::stop() {

    }

    NetAddr::net_addr_sptr_t_ TCPClient::getPeerAddr() {
        return rocket::NetAddr::net_addr_sptr_t_();
    }

    NetAddr::net_addr_sptr_t_ TCPClient::getLocalAddr() {
        return rocket::NetAddr::net_addr_sptr_t_();
    }
}



