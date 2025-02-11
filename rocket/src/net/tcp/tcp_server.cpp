//
// Created by baitianyu on 25-2-10.
//
#include "net/tcp/tcp_server.h"

namespace rocket {

    TCPServer::TCPServer(NetAddr::ptr local_addr) : m_local_addr(local_addr) {
        m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
        m_main_event_loop = EventLoop::GetCurrentEventLoop();
        m_io_thread_pool = std::make_unique<IOThreadPool>(MAX_THREAD_POOL_SIZE);
        m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
        m_dispatcher = RPCDispatcher::GetCurrentRPCDispatcher();
        init();
    }

    TCPServer::~TCPServer() {
        DEBUGLOG("~TCPServer");
    }

    void TCPServer::addServlet(const std::string &uri, Servlet::ptr slt) {
        m_dispatcher->addServlet(uri, slt);
    }

    void TCPServer::start() {
        m_io_thread_pool->start();
        m_main_event_loop->loop();
    }

    void TCPServer::init() {
        m_clear_client_timer_event = std::make_shared<TimerEventInfo>(TIMER_EVENT_INTERVAL,
                                                                      true,
                                                                      std::bind(&TCPServer::clearClientTimerFunc,
                                                                                this));
        m_main_event_loop->addTimerEvent(m_clear_client_timer_event);
        m_listen_fd_event->listen(FDEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);
    }

    void TCPServer::onAccept() {
        auto ret = m_acceptor->accept();
        auto client_fd = ret.first;
        auto peer_addr = ret.second;
        auto &io_thread = m_io_thread_pool->getIOThread();
        auto connection = std::make_shared<TCPConnection>(
                io_thread->getEventLoop(),
                m_local_addr,
                peer_addr,
                client_fd,
                MAX_TCP_BUFFER_SIZE,
                m_dispatcher,
                TCPConnectionType::TCPConnectionByServer
        );
        connection->setState(Connected);
        m_client_connections.insert(connection);
        // 轮询添加到线程池中的线程中
        INFOLOG("TcpServer succeed get client, fd: %d, peer addr: %s, now client counts: %d", client_fd,
                peer_addr->toString().c_str(), m_client_connections.size());
    }

    void TCPServer::clearClientTimerFunc() {
        auto iter = m_client_connections.begin();
        for (; iter != m_client_connections.end();) {
            if ((*iter) != nullptr && (*iter).use_count() > 0 && (*iter)->getState() == TCPState::Closed) {
                // erase的返回值是指向被删除元素的后继元素的迭代器
                iter = m_client_connections.erase(iter);
            } else {
                iter++;
            }
        }
    }

    std::unique_ptr<IOThreadPool> &TCPServer::getIOThreadPool() {
        return m_io_thread_pool;
    }
}

