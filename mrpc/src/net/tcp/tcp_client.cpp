//
// Created by baitianyu on 25-2-10.
//
#include <unistd.h>
#include "net/tcp/tcp_client.h"
#include "event/fd_event_pool.h"
#include "common/log.h"
#include "common/error_code.h"

namespace mrpc {

    TCPClient::TCPClient(NetAddr::ptr peer_addr, EventLoop::ptr specific_eventloop)
            : m_peer_addr(peer_addr), m_event_loop(specific_eventloop) {
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
                nullptr,
                peer_addr,
                m_client_fd,
                MAX_TCP_BUFFER_SIZE,
                nullptr,
                TCPConnectionType::TCPConnectionByClient
        );
    }

    TCPClient::~TCPClient() {
        if (m_client_fd > 0) {
            m_event_loop->deleteEpollEvent(m_fd_event);
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
                        close(m_client_fd);
                        m_client_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                    }
                    m_event_loop->deleteEpollEvent(m_fd_event);
                    // 无论成功失败都得调用回调，否则外面无法获取出错的状态，即无法调用获取出错，或者获取成功的状态
                    if (done) {
                        done();
                    }
                });
                m_event_loop->addEpollEvent(m_fd_event);
                if (!m_event_loop->LoopStopFlag()) {
                    m_event_loop->loop();
                } else {
                    m_event_loop->setLoopStopFlag();
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

    void TCPClient::sendRequest(const HTTPRequest::ptr &request, const std::function<void(HTTPRequest::ptr)> &done) {
        m_connection->pushSendMessage(request, done);
        m_connection->listenWrite(); // 监听可写的时候写入就行了
    }

    void
    TCPClient::recvResponse(const std::string &msg_id, const std::function<void(HTTPResponse::ptr)> &done) {
        m_connection->pushReadMessage(msg_id, done);
        m_connection->listenRead(); // 去监听可读事件
    }

    NetAddr::ptr TCPClient::getPeerAddr() {
        return m_peer_addr;
    }

    NetAddr::ptr TCPClient::getLocalAddr() {
        return m_local_addr;
    }

    void TCPClient::setPeerAddr(NetAddr::ptr new_peer_addr) {
        m_peer_addr = new_peer_addr;
    }

    int TCPClient::getConnectErrorCode() const {
        return m_connect_err_code;
    }

    std::string TCPClient::getConnectErrorInfo() {
        return m_connect_err_info;
    }

    void TCPClient::initLocalAddr() {
        // 由于是client，其地址是系统随机分配的，所以可以读取sockfd来读取到其分配到的地址
        sockaddr_in local_addr;
        socklen_t len = sizeof(local_addr);
        int ret = getsockname(m_client_fd, (sockaddr *) (&local_addr), &len);
        if (ret != 0) {
            ERRORLOG("initLocalAddr error, getsockname error. errno = %d, error = %s", errno, strerror(errno));
            return;
        }
        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }

    int TCPClient::getClientFD() const {
        return m_client_fd;
    }

    EventLoop::ptr TCPClient::getEventLoop() {
        return m_event_loop;
    }
}