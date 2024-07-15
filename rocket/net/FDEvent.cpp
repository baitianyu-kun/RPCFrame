//
// Created by baitianyu on 7/15/24.
//
#include <cstring>
#include <fcntl.h>
#include "net/FDEvent.h"

namespace rocket {

    FDEvent::FDEvent(int fd) : m_fd(fd) {
        // 初始化epoll events
        memset(&m_listen_event, 0, sizeof(m_listen_event));
    }

    rocket::FDEvent::FDEvent() {
        memset(&m_listen_event, 0, sizeof(m_listen_event));
    }

    FDEvent::~FDEvent() {

    }

    int FDEvent::setNonBlock() {
        int old_option = fcntl(m_fd, F_GETFL);
        int new_option = old_option | O_NONBLOCK;
        fcntl(m_fd, F_SETFL, new_option);
        return old_option;
    }

    std::function<void()> FDEvent::handler(FDEvent::TriggerEventType event_type) {
        // 如果传进来的是read，则调read的回调，其他的也一样
        if (event_type == TriggerEventType::IN_EVENT)
            return m_read_callback;
        else if (event_type == TriggerEventType::OUT_EVENT)
            return m_write_callback;
        else if (event_type == TriggerEventType::ERROR_EVENT)
            return m_error_callback;
    }

    void FDEvent::listen(FDEvent::TriggerEventType event_type, std::function<void()> callback,
                         std::function<void()> error_callback /*null ptr*/) {
        // 需要修改: Clang-Tidy:参数'error_callback'在每次调用时都被复制，但只用作const引用;考虑将其作为const引用
        if (event_type == TriggerEventType::IN_EVENT) {
            // epoll_ctl中的EPOLL_CTL_ADD是指设置将fd加到epoll fd的中，只对epoll fd操作，或者设置其他例如EPOLL_CTL_MOD修改、EPOLL_CTL_DEL删除
            // 而这里的events代表事件类型，对epoll event进行操作，例如EPOLLIN，EPOLLOUT，或者EPOLLERR
            // 设置类型就是events |= EPOLLIN，取消就是events &= ~EPOLLIN
            m_listen_event.events |= EPOLLIN;
            m_read_callback = callback;
        } else if (event_type == TriggerEventType::OUT_EVENT) {
            m_listen_event.events |= EPOLLOUT;
            m_write_callback = callback;
        }
        if (error_callback != nullptr) {
            m_error_callback = error_callback;
        }
        // 当前的epoll event设置为当前的对象
        m_listen_event.data.ptr = this;
    }

    void FDEvent::cancel_listen(FDEvent::TriggerEventType event_type) {
        event_type == TriggerEventType::IN_EVENT ? m_listen_event.events &= (~EPOLLIN)
                                                 : m_listen_event.events &= (~EPOLLOUT);
    }

    WakeUpFDEvent::WakeUpFDEvent(int fd) : FDEvent(fd) {

    }

    WakeUpFDEvent::~WakeUpFDEvent() {

    }

    void WakeUpFDEvent::wakeup() {

    }
}

