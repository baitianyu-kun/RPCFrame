//
// Created by baitianyu on 8/30/24.
//
#include "net/tcp/net_addr.h"
#include "coroutine/coroutine.h"
#include "common/log.h"
#include "net/tcp/tcp_acceptor.h"
#include "unistd.h"

using namespace rocket;

void accept_func() {
    auto cur = Coroutine::GetCurrentCoroutine();
    NetAddr::net_addr_sptr_t_ m_local_addr = std::make_shared<IPNetAddr>("127.0.0.1", 22227);
    auto m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
    auto m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
    m_listen_fd_event->listen(FDEvent::IN_EVENT, [cur, m_acceptor, m_listen_fd_event]() {
        DEBUGLOG("[MAIN] now accept begin to resume");
        m_listen_fd_event->cancel_listen(FDEvent::IN_EVENT);
        EventLoop::GetCurrentEventLoop()->deleteEpollEvent(m_listen_fd_event);
        Coroutine::Resume(cur);
    });
    EventLoop::GetCurrentEventLoop()->addEpollEvent(m_listen_fd_event);
    DEBUGLOG("[COR %d] now accept begin to yield", cur->getCorId());
    Coroutine::Yield();
    DEBUGLOG("[COR %d] now resume to accept",cur->getCorId());
    m_acceptor->accept();
    DEBUGLOG("[COR %d] accept success", cur->getCorId());
}

int main() {

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);

    // 构建协程
    int stack_size = 128 * 1024;
    char *sp = reinterpret_cast<char *>(malloc(stack_size));
    auto cor = std::make_shared<rocket::Coroutine>(stack_size, sp, accept_func);

    EventLoop::GetCurrentEventLoop();

    DEBUGLOG("[MAIN] main coroutine, resume to cor [%d]", cor->getCorId());
    Coroutine::Resume(cor.get());
    DEBUGLOG("[MAIN] back to main coroutine");
    EventLoop::GetCurrentEventLoop()->loop();
}