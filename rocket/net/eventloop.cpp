//
// Created by baitianyu on 7/14/24.
//

#include <sys/socket.h>
#include "net/eventloop.h"
#include "common/log.h"
#include "common/util.h"

namespace rocket {

    // 每个线程只能有一个event loop
    // ThreadLocal用于保存某个线程共享变量：对于同一个static ThreadLocal，
    // 不同线程只能从中get，set，remove自己的变量，而不会影响其他线程的变量。
    static thread_local EventLoop *t_current_event_loop = nullptr;

    rocket::EventLoop::EventLoop() {
        if (t_current_event_loop != nullptr) {
            ERRORLOG("failed to create event loop, this thread has created event loop");
            exit(1);
        }
        m_pid = getThreadId();
        INFOLOG("succeed create event loop in thread %d",m_pid);

    }

    rocket::EventLoop::~EventLoop() {

    }

    void rocket::EventLoop::loop() {

    }

    void rocket::EventLoop::wakeup() {

    }

    void rocket::EventLoop::stop() {

    }
}

