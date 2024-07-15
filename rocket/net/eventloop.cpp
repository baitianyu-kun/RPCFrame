//
// Created by baitianyu on 7/14/24.
//

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cstring>
#include <sys/eventfd.h>
#include "net/eventloop.h"
#include "common/log.h"
#include "common/util.h"

namespace rocket {

    // 每个线程只能有一个event loop
    // ThreadLocal用于保存某个线程共享变量：对于同一个static ThreadLocal，
    // 不同线程只能从中get，set，remove自己的变量，而不会影响其他线程的变量。
    static thread_local EventLoop *t_current_event_loop = nullptr;
    \

    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    rocket::EventLoop::EventLoop() {
        if (t_current_event_loop != nullptr) {
            ERRORLOG("failed to create event loop, this thread has created event loop");
            exit(0);
        }
        m_pid = getThreadId();
        // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero;
        // 之前需要确定有多少个需要加入，现在新版本size被忽略，因为kernel使用了可扩展的data structure
        m_epoll_fd = epoll_create(10);

        if (m_epoll_fd == -1) {
            ERRORLOG("failed to create event loop, epoll create error, error code: [%d]", errno);
            exit(0);
        }

        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0) {
            ERRORLOG("failed to create event loop, epoll create error, error code: [%d]", errno);
            exit(0);
        }

        // 创建epoll event去监听wakeup的读取事件
        epoll_event tmp_event;
        tmp_event.events = EPOLLIN; // 读取事件
        // 添加到哪个epollfd里，要添加的fd是什么，要添加还是删除，要添加什么事件
        int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wakeup_fd, &tmp_event);
        if (ret == -1) {
            ERRORLOG("failed to create event loop, EPOLL_CTL_ADD error, error code: [%d]", errno);
            exit(0);
        }

        INFOLOG("succeed create event loop in thread %d", m_pid);
        t_current_event_loop = this;
    }

    rocket::EventLoop::~EventLoop() {

    }

    void rocket::EventLoop::loop() {
        m_is_looping = true;
        while (!m_stop_flag) {
            ScopeMutext<Mutex> lock(m_mutex);
            // 取出队列
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();

            // 开始处理任务队列
            while (!tmp_tasks.empty()) {
                auto task = tmp_tasks.front();
                tmp_tasks.pop();
                if (task) {
                    task();
                }
            }

            // 处理完队列之后，开始开启epoll
            auto timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];
            auto ret = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, g_epoll_max_timeout);

            if (ret < 0) {
                ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
            } else {
                // 处理就绪时间，由于epoll wait就绪时候会复制到result events中，ret就是就绪的数量
                for (int i = 0; i < ret; ++i) {
                    auto &trigger_event = result_events[i];
                    // epoll data是union，ptr指定和fd相关的用户数据，fd指定目标描述符，由于union所以二者只能使用1个
                    // 所以为了访问数据，只能用ptr，可以强制转换为自己的对象FDEvent
                    auto fd_event = reinterpret_cast<FDEvent *>(trigger_event.data.ptr);
                    if (fd_event == nullptr) {
                        ERRORLOG("fd event = nullptr, continue");
                        continue;
                    }
                    // 是读取事件的话
                    if (trigger_event.events & EPOLLIN) {

                    }
                }
            }

        }
    }

    void rocket::EventLoop::wakeup() {

    }

    void rocket::EventLoop::stop() {
        m_stop_flag = true;
    }

    void EventLoop::dealWakeUp() {

    }

    void EventLoop::addTask(std::function<void()> callback, bool is_wake_up) {
        // class scoped_lock;是C++ 17才出现的
        ScopeMutext<Mutex> lock(m_mutex);
        m_pending_tasks.push(callback);
        lock.unlock();
        if (is_wake_up) {
            // TODO add wake up func
        }
    }
}

