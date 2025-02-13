//
// Created by baitianyu on 2/12/25.
//
#include "rpc/rpc_publish_listener.h"
#include "common/log.h"

namespace mrpc {

    PublishListenerRunner::PublishListenerRunner(NetAddr::ptr local_addr) :
            m_local_addr(local_addr) {
        m_main_event_loop = EventLoop::GetCurrentEventLoop();
        m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
        m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
        m_dispatcher = RPCDispatcher::GetCurrentRPCDispatcher();
        init();
    }

    PublishListenerRunner::~PublishListenerRunner() {
        DEBUGLOG("~PublishListenerRunner");
    }

    void PublishListenerRunner::addServlet(const std::string &uri, Servlet::ptr slt) {
        m_dispatcher->addServlet(uri, slt);
    }

    void PublishListenerRunner::addServlet(const std::string &uri, CallBacksServlet::callback cb) {
        m_dispatcher->addServlet(uri, cb);
    }

    void PublishListenerRunner::start() {
        m_main_event_loop->loop();
    }

    void PublishListenerRunner::init() {
        m_listen_fd_event->listen(FDEvent::IN_EVENT, std::bind(&PublishListenerRunner::onAccept, this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);
    }

    void PublishListenerRunner::onAccept() {
        auto ret = m_acceptor->accept();
        auto client_fd = ret.first;
        auto peer_addr = ret.second;
        m_connection.reset(); // 保存一下connection，防止在runner执行前被析构
        m_connection = std::make_shared<TCPConnection>(
                m_main_event_loop,
                m_local_addr,
                peer_addr,
                client_fd,
                MAX_TCP_BUFFER_SIZE,
                m_dispatcher,
                TCPConnectionType::TCPConnectionByServer
        );
        m_connection->setState(Connected);
    }

    PublishListener::PublishListener(NetAddr::ptr local_addr, PublishListener::callback call_back) {
        m_runner = std::make_shared<PublishListenerRunner>(local_addr);
        m_runner->addServlet(RPC_REGISTER_PUBLISH_PATH, call_back);
        pthread_create(&m_thread, nullptr, PublishListener::runner, m_runner.get());
    }

    PublishListener::~PublishListener() {
        DEBUGLOG("~PublishListener");
    }

    void *PublishListener::runner(void *arg) {
        auto runner = reinterpret_cast<PublishListenerRunner *>(arg);
        runner->start();
        return nullptr;
    }
}