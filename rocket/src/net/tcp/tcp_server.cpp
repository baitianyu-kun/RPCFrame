//
// Created by baitianyu on 7/20/24.
//
#include "net/tcp/tcp_server.h"

namespace rocket {
    rocket::TCPServer::TCPServer(NetAddr::net_addr_sptr_t_ local_addr) : m_local_addr(local_addr) {
        init();
        INFOLOG("rocket TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
    }

    TCPServer::~TCPServer() {

    }

    void TCPServer::start() {
        m_io_thread_pool->start();
        // main event loop是主线程的epoll，但是每个子线程也各自拥有一个epoll，即各自拥有一个eventloop，整个是个这样的模式
        // 是一个主从reactor模式
        m_main_event_loop->loop();
    }

    // 初始化各种东西
    void TCPServer::init() {
        // 接受器
        m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
        m_main_event_loop = std::move(std::unique_ptr<EventLoop>(EventLoop::GetCurrentEventLoop()));
        m_io_thread_pool = std::move(std::unique_ptr<IOThreadPool>(new IOThreadPool(MAX_THREAD_POOL_SIZE)));
        // 获取监听套接字，监听到上面有可读事件就执行onAccept事件
        m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
        // 需要传递一个this指针过去指明对象，也不能TCPServer::onAccept，因为onAccept不是static的，所以前面加&
        // bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址。
        // 必须显式地指定&Base::diplay_sum，因为编译器不会将对象的成员函数隐式转换成函数指针，所以必须在Base::display_sum前添加&；
        // 使用对象成员函数的指针时，必须要知道该指针属于哪个对象，因此第二个参数为对象的地址 &base或者this；
        m_listen_fd_event->listen(FDEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);

        m_clear_client_timer_event = std::make_shared<TimerEventInfo>(TIMER_EVENT_INTERVAL, true,
                                                                      std::bind(&TCPServer::clearClientTimerFunc,
                                                                                this));
        m_main_event_loop->addTimerEvent(m_clear_client_timer_event);
    }

    void TCPServer::onAccept() {
        auto ret = m_acceptor->accept();
        auto client_fd = ret.first;
        auto peer_addr = ret.second;
        m_client_counts++; // 已经连接的子线程个数

        // 轮询插入io thread中
        auto &io_thread = m_io_thread_pool->getIOThread();
        // 创建connection处理读写
        auto connection = std::make_shared<TCPConnection>(
                io_thread->getEventLoop(),
                client_fd,
                MAX_TCP_BUFFER_SIZE,
                peer_addr,
                m_local_addr
        );
        connection->setState(Connected);
        m_client_connection.insert(connection);
        // 轮询添加到线程池中的线程中
        INFOLOG("TcpServer succ get client, fd: %d, peer addr: %s, now client counts: %d", client_fd,
                peer_addr->toString().c_str(), m_client_counts);
    }

    void TCPServer::clearClientTimerFunc() {

    }
}


