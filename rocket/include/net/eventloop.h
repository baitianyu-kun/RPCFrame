//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_EVENTLOOP_H
#define RPCFRAME_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "common/mutex.h"
#include "net/FDEvent.h"

namespace rocket {

    class EventLoop {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void wakeup();

        void stop();

        void addTask(std::function<void()> callback, bool is_wake_up = false);
    private:
        void dealWakeUp();
    private:
        // event loop每个线程只能有一个，线程号
        pid_t m_pid;
        // 唤醒epoll wait
        int m_wakeup_fd{0};
        // epoll的fd
        int m_epoll_fd{0};
        // 当前epoll监听的所有fds
        std::set<int> m_listen_fds;
        // loop循环的stop标志
        bool m_stop_flag{false};
        bool m_is_looping {false};
        // 所有待执行的任务队列
        // 包括普通函数、Lambda表达式、函数指针、以及其它函数对象
        // 括号外面是返回值类型，内部是参数类型
        std::queue<std::function<void()>> m_pending_tasks;
        // 需要加锁
        Mutex m_mutex;
    };

}

#endif //RPCFRAME_EVENTLOOP_H
