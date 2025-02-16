//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_EVENTLOOP_H
#define RPCFRAME_EVENTLOOP_H

#include <set>
#include <functional>
#include <queue>
#include <memory>
#include <cassert>
#include "common/mutex.h"
#include "event/fd_event.h"
#include "event/wake_up_fd_event.h"
#include "event/timer_queue.h"

namespace mrpc {

    class EventLoop {
    public:
        using ptr = std::shared_ptr<EventLoop>;

    public:
        // 单例模式，一个线程一个event loop
        static thread_local ptr t_current_event_loop;

        static ptr GetCurrentEventLoop();

    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void wakeup();

        void stop();

        void addEpollEvent(FDEvent::ptr fd_event_s_ptr);

        void deleteEpollEvent(FDEvent::ptr fd_event_s_ptr);

        bool isInLoopThread();

        bool LoopStopFlag();

        void setLoopStopFlag();

        void addTask(std::function<void()> callback, bool is_wake_up = false);

        TimerId addTimerEvent(TimerQueue::TimerCallback cb, Timestamp when, double interval);

        void deleteTimerEvent(TimerId timerId);

    private:
        void dealWakeup();

        void initWakeUpFDEvent();

        void initTimer();

    private:
        // event loop每个线程只能有一个，线程号
        pid_t m_pid;
        // wake up event
        WakeUpFDEvent::ptr m_wakeup_fd_event{nullptr};
        // epoll的fd
        int m_epoll_fd{0};
        // 当前epoll监听的所有fds
        std::set<int> m_listen_fds;
        // loop循环的stop标志
        bool m_stop_flag{false};
        // 所有待执行的任务队列
        // 包括普通函数、Lambda表达式、函数指针、以及其它函数对象
        // 括号外面是返回值类型，内部是参数类型
        std::queue<std::function<void()>> m_pending_tasks;
        // 需要加锁
        Mutex m_mutex;
        // time定时任务
        TimerQueue::ptr m_timer;
    };

}

#endif //RPCFRAME_EVENTLOOP_H
