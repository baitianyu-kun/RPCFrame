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
            initLocalAddr();
            if (done) {
                done();
            }
        } else if (ret == -1) {
            // TODO CHANGE HERE, ADD CONNECT METHOD
            // TODO AND initLocalAddr

            // epoll监听可写事件，就是证明已经连接成功了，此时缓冲区可写，为什么不是可读事件呢？因为此时没有可读的
            // 正在建立连接，可以在epoll中继续建立连接，并判断继续建立的这个连接的错误码做出相应的回复

            // 这里的done就是去注册了tcp client的可写可读事件，如果走到这里的话，需要去在epoll里去监听
            // 连接状态，如果连接成功就去设置done，即监听可写事件listenWrite，如果write还没触发，那么直接就执行
            // =============================================
            // 处理完后需要取消监听写事件，否则会一直触发
            // m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
            // m_event_loop->addEpollEvent(m_fd_event);
            // =============================================
            // 又把写事件给取消监听了，所以就会出现时序问题，所以就应该连接成功后保存个状态，然后把listenWrite
            // 取消，防止一直触发，然后根据这个状态，如果是成功的话就应该调用done，即去注册listenWrite，否则
            // 什么也不干

            if (errno == EINPROGRESS) {
                m_fd_event->listen(FDEvent::OUT_EVENT, [this, done]() {
                    int error = 0;
                    socklen_t error_len = sizeof(error);
                    getsockopt(m_client_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                    bool is_connect_success = false;
                    if (error == 0) {
                        initLocalAddr();
                        DEBUGLOG("connect [%s] sussess", m_peer_addr->toString().c_str());
                        is_connect_success = true;
                    } else {
                        ERRORLOG("connect errror, errno=%d, error=%s", errno, strerror(errno));
                    }
                    // 处理完后需要取消监听写事件，否则会一直触发
                    m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
                    m_event_loop->addEpollEvent(m_fd_event);
                    if (is_connect_success && done) {
                        // 还得设置已经连接，或者是在上面设置都行，否则客户端会一直报错说connection的状态不是Connected
                        m_connection->setState(Connected);
                        done();
                    }
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

    // 将message对象写入到tcp connection中的out buffer中，同时done函数也要写入进去，发送完后就可以找到对应的回调函数去执行
    // 然后启动connection可写事件就可以
    void TCPClient::writeMessage(const AbstractProtocol::abstract_pro_sptr_t_ &message,
                                 const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done) {
        m_connection->pushSendMessage(message, done);
        m_connection->listenWrite(); // 监听可写的时候写入就行了
    }

    // 1. 监听可读事件
    // 2. 从buffer里面读取，并decode得到message对象，如果msg id相等的话则读取成功，执行其回调done
    void TCPClient::readMessage(const std::string &msg_id,
                                const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done) {
        m_connection->pushReadMessage(msg_id, done);
        m_connection->listenRead(); // 去监听可读事件
    }

    void TCPClient::stop() {
        m_event_loop->stop();
    }

    NetAddr::net_addr_sptr_t_ TCPClient::getPeerAddr() {
        return m_peer_addr;
    }

    NetAddr::net_addr_sptr_t_ TCPClient::getLocalAddr() {
        return m_local_addr;
    }

    void TCPClient::initLocalAddr() {
        // 由于是client，其地址是系统随机分配的，所以可以读取sockfd来读取到其分配到的地址
        sockaddr_in local_addr;
        socklen_t len = sizeof(local_addr);
        int ret = getsockname(m_client_fd, (sockaddr *) (&local_addr), &len);
        if (ret != 0) {
            ERRORLOG("initLocalAddr error, getsockname error. errno=%d, error=%s", errno, strerror(errno));
            return;
        }
        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }

}



