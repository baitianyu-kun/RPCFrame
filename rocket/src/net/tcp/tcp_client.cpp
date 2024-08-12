//
// Created by baitianyu on 7/22/24.
//
#include <unistd.h>
#include "net/tcp/tcp_client.h"
#include "common/error_code.h"
#include "net/coder/tinypb/tinypb_coder.h"
#include "net/coder/http/http_coder.h"

namespace rocket {
    rocket::TCPClient::TCPClient(NetAddr::net_addr_sptr_t_ peer_addr,
                                 ProtocolType protocol/*ProtocolType::TinyPB_Protocol*/) : m_peer_addr(peer_addr),
                                                                                           m_protocol_type(
                                                                                                   protocol) {
        m_event_loop = EventLoop::GetCurrentEventLoop();
        m_client_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
        if (m_client_fd < 0) {
            ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
            return;
        }
        m_fd_event = FDEventPool::GetFDEventPool()->getFDEvent(m_client_fd);
        m_fd_event->setNonBlock();
        if (m_protocol_type == ProtocolType::HTTP_Protocol) {
            m_coder = std::make_shared<HTTPCoder>();
        } else if (m_protocol_type == ProtocolType::TinyPB_Protocol) {
            m_coder = std::make_shared<TinyPBCoder>();
        }
        // 作为client的情况没有本地监听地址local addr
        m_connection = std::make_shared<TCPConnection>(
                m_event_loop,
                m_client_fd,
                MAX_TCP_BUFFER_SIZE,
                peer_addr,
                nullptr,
                m_coder,
                nullptr,
                TCPConnectionByClient,
                m_protocol_type
        );
    }

    TCPClient::~TCPClient() {
        // 需要把刚刚主动连接的socket给销毁掉
        if (m_client_fd > 0) {
            close(m_client_fd);
        }
        DEBUGLOG("~TCPClient");
    }

    void TCPClient::connect(std::function<void()> done) {
        int ret = ::connect(m_client_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockAddrLen());
        if (ret == 0) {
            DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
            m_connection->setState(Connected);
            initLocalAddr();
            if (done) {
                done();
            }
        } else if (ret == -1) {
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

            // errno = EINPROGRESS的时候可以用getsockopt，也可以再调用一遍connect方法
            // 并处理连接出错的情况，例如服务端没有开监听，客户端就去连接的情况

            // 需要加入，在connect error之后自动重连
            if (errno == EINPROGRESS) {
                m_fd_event->listen(FDEvent::OUT_EVENT, [this, done]() {
                    int ret = ::connect(m_client_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockAddrLen());
                    if ((ret == 0) || (ret < 0 && errno == EISCONN)) {
                        // 连接成功
                        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                        m_connection->setState(Connected);
                        initLocalAddr();
                    } else {
                        if (errno == ECONNREFUSED) {
                            m_connect_err_code = ERROR_PEER_CLOSED;
                            m_connect_err_info = "connect refused, sys error = " + std::string(strerror(errno));
                        } else {
                            m_connect_err_code = ERROR_FAILED_CONNECT;
                            m_connect_err_info = "connect unknown error, sys error = " + std::string(strerror(errno));
                        }
                        ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
                        // 出错了，关闭该套接字并重新申请一个
                        // 通过关闭旧的套接字并重新申请一个新的套接字，可以清理之前连接时可能存在的问题，如连接状态、缓冲区等，从而在重新连接时避免潜在的影响
                        // 旧的套接字可能处于错误状态，包含之前连接尝试的残留信息，这可能导致后续的连接尝试出现意外行为
                        // 新的连接尝试可能会受到之前连接错误的影响

                        // 若connect失败则该套接字不再可用，必须关闭，我们不能对这样的套接字再次调用connect函数。
                        // 在每次connect失败后，都必须close当前套接字描述符并重新调用socket。

                        // 网络连接断开后，客户端尝试直接使用connect()函数重连，无论重复多少次connect()函数都只会返回-1（无论服务器端本身是否已经可以被重新连接）
                        close(m_client_fd);
                        m_client_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                    }
                    // 连接完成后去掉可写时间的监听，不然会一直触发
                    // m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
                    // m_event_loop->addEpollEvent(m_fd_event);
                    // 或者m_event_loop->deleteEpollEvent(m_fd_event);这两种办法都可以
                    m_event_loop->deleteEpollEvent(m_fd_event);
                    // 无论成功失败都得调用回调，否则外面无法获取出错的状态，即无法调用获取出错，或者获取成功的状态
                    if (done) {
                        done();
                    }
                });
                m_event_loop->addEpollEvent(m_fd_event);
                // 添加完事件后记得开启loop循环
                if (!m_event_loop->LoopStopFlag()) {
                    m_event_loop->loop();
                }
            } else {
                ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
                // 需要返回具体的错误码
                m_connect_err_code = ERROR_FAILED_CONNECT;
                m_connect_err_info = "connect error, sys error = " + std::string(strerror(errno));
                // 无论成功失败都得调用回调，否则外面无法获取出错的状态，即无法调用获取出错，或者获取成功的状态
                if (done) {
                    done();
                }
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
        int ret = getsockname(m_client_fd, (sockaddr * )(&local_addr), &len);
        if (ret != 0) {
            ERRORLOG("initLocalAddr error, getsockname error. errno = %d, error = %s", errno, strerror(errno));
            return;
        }
        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }

    int TCPClient::getConnectErrorCode() const {
        return m_connect_err_code;
    }

    std::string TCPClient::getConnectErrorInfo() {
        return m_connect_err_info;
    }

    EventLoop::event_loop_sptr_t_ TCPClient::getEventLoop() {
        return m_event_loop;
    }

    TCPConnection::tcp_connection_sptr_t_ &TCPClient::getConnectionRef() {
        return m_connection;
    }

}



