//
// Created by baitianyu on 7/14/24.
//

#include <sys/epoll.h>
#include <cstring>
#include <utility>
#include "event/eventloop.h"
#include "common/log.h"
#include "common/util.h"

// 使用do{...}while(0)构造后的宏定义不会受到大括号、分号等的影响，总是会按你期望的方式调用运行。
#define ADD_OR_MODIFY_TO_EPOLL() \
    do{                \
        auto it = m_listen_fds.find(fd_event->getFD());\
        int op = it != m_listen_fds.end() ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;\
        auto tmp_epoll_event = fd_event->getEpollEvent();\
        INFOLOG("epoll_event.events = %d", (int) tmp_epoll_event.events);\
        int ret = epoll_ctl(m_epoll_fd, op, fd_event->getFD(), &tmp_epoll_event);\
        if (ret == -1) {\
            ERRORLOG("failed epoll_ctl when add fd, errno [%d], error [%s], op [%d]", errno, strerror(errno),op);\
            ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd_event->getFD(), &tmp_epoll_event);                         \
        }                        \
        m_listen_fds.emplace(fd_event->getFD());\
        DEBUGLOG("add event success, fd[%d]", fd_event->getFD());\
    }while(0)

#define DELETE_FROM_EPOLL() \
    do{                \
        auto it = m_listen_fds.find(fd_event->getFD());\
        if (it == m_listen_fds.end()) {                \
            return;\
        }\
        int op = EPOLL_CTL_DEL;\
        int ret = epoll_ctl(m_epoll_fd, op, fd_event->getFD(), nullptr);\
        if (ret == -1) {\
            ERRORLOG("failed epoll_ctl when delete fd, errno [%d], error [%s]", errno, strerror(errno));\
        }\
        m_listen_fds.erase(fd_event->getFD());\
        DEBUGLOG("delete event success, fd [%d]", fd_event->getFD());\
    }while(0)

namespace mrpc {
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    thread_local EventLoop::ptr EventLoop::t_current_event_loop =
            std::make_shared<EventLoop>();

    EventLoop::ptr EventLoop::GetCurrentEventLoop() {
        return t_current_event_loop;
    }

    mrpc::EventLoop::EventLoop() {
        // 设置当前线程的id
        m_pid = getThreadId();
        // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero;
        // 之前需要确定有多少个需要加入，现在新版本size被忽略，因为kernel使用了可扩展的data structure
        m_epoll_fd = epoll_create(10);
        if (m_epoll_fd == -1) {
            ERRORLOG("failed to create event loop, epoll create error, error code [%d]", errno);
            exit(0);
        }
        // 初始化唤醒fd
        initWakeUpFDEvent();
        // 初始化定时任务
        initTimer();
        INFOLOG("succeed create event loop in thread [%d]", m_pid);
    }

    mrpc::EventLoop::~EventLoop() {
        close(m_epoll_fd);
    }

    void mrpc::EventLoop::loop() {
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
            // 每次while的时候，都会开始将这几个event和epollfd进行监听，
            // 当要退出时候，while循环结束，等epoll wait结束阻塞后，下一次就不在继续监听了
            // 实现了退出。所以这里while循环结束后，epoll wait可能还在阻塞，所以
            // 需要在stop的时候给epoll wait进行wake up
            auto ret = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
            if (ret < 0) {
                ERRORLOG("epoll_wait error, errno [%d], error [%s]", errno, strerror(errno));
            } else {
                // 处理就绪时间，由于epoll wait就绪时候会复制到result events中，ret就是就绪的数量
                // 就绪了实际上就是把callback注册到task中，方便loop下一次进行处理
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
                        // 往task中加入event的callback任务
                        // handler就是去响应事件，随后去调用预先设置好的callback
                        addTask(fd_event->handler(FDEvent::IN_EVENT));
                    }
                    if (trigger_event.events & EPOLLOUT) {
                        addTask(fd_event->handler(FDEvent::OUT_EVENT));
                    }
                    if (trigger_event.events & EPOLLERR) {
                        DEBUGLOG("fd %d trigger EPOLLERR event", fd_event->getFD());
                        // important
                        // 需要删除出错的fd，由于此处的fd event不是new出来的
                        // 所以需要给智能指针指定一个空的删除器，即shared ptr析构的时候
                        // 不去delete这个不是new出来的东西。这里用lambda表达式实现
                        deleteEpollEvent(std::shared_ptr<FDEvent>(fd_event, [](FDEvent *) {}));
                        auto error_callback = fd_event->handler(FDEvent::ERROR_EVENT);
                        if (error_callback != nullptr) {
                            DEBUGLOG("fd %d add error callback", fd_event->getFD());
                            addTask(error_callback);
                        }
                    }
                }
            }
        }
    }

    // 执行完任务队列以后，loop会阻塞在epoll wait中，无法继续执行下一次任务队列
    // 所以需要wake up epoll wait打破阻塞，去完成下一次任务队列
    void mrpc::EventLoop::wakeup() {
        INFOLOG("WAKE UP, %d", getThreadId());
        m_wakeup_fd_event->wakeup();
    }

    void EventLoop::dealWakeup() {

    }

    // 如果要停止，此时阻塞在epoll wait中，需要打破阻塞，然后while检测到stop flag，所以可以直接退出
    // 要记得wakeup，否则会暂时阻塞在event loop里面没人唤醒
    void mrpc::EventLoop::stop() {
        // 先判断，否则可能会在不同的地方停止两次，例如在client中停止一次，设置停止符并wakeup，然后有可能
        // 在io thread中再次停止一次，那么此时wakeup就不会执行了，因为eventloop已经停止了
        if (!m_stop_flag) {
            m_stop_flag = true;
            wakeup();
        }
    }

    void EventLoop::initWakeUpFDEvent() {
        m_wakeup_fd_event = std::make_shared<WakeUpFDEvent>();
        addEpollEvent(m_wakeup_fd_event);
    }

    void EventLoop::addTask(std::function<void()> callback, bool is_wake_up /* false */) {
        // class scoped_lock;是C++ 17才出现的
        ScopeMutext<Mutex> lock(m_mutex);
        m_pending_tasks.push(callback);
        lock.unlock();
        if (is_wake_up) {
            wakeup();
        }
    }

    void EventLoop::addEpollEvent(FDEvent::ptr fd_event_s_ptr) {
        auto fd_event = fd_event_s_ptr.get();
        if (isInLoopThread()) {
            ADD_OR_MODIFY_TO_EPOLL();
        } else {
            auto callback = [this, fd_event]() {
                ADD_OR_MODIFY_TO_EPOLL();
            };
            addTask(callback, true);
        }
    }

    void EventLoop::deleteEpollEvent(FDEvent::ptr fd_event_s_ptr) {
        auto fd_event = fd_event_s_ptr.get();
        if (isInLoopThread()) {
            DELETE_FROM_EPOLL();
        } else {
            auto callback = [this, fd_event]() {
                DELETE_FROM_EPOLL();
            };
            addTask(callback, true);
        }
    }

    // 判断是否是当前线程在loop中
    bool EventLoop::isInLoopThread() {
        return m_pid == getThreadId();
    }

    bool EventLoop::LoopStopFlag() {
        return m_stop_flag;
    }

    void EventLoop::setLoopStopFlag() {
        m_stop_flag = false;
    }

    void EventLoop::initTimer() {
        m_timer = std::make_shared<TimerQueue>();
        addEpollEvent(m_timer);
    }

    TimerId EventLoop::addTimerEvent(TimerQueue::TimerCallback cb, Timestamp when, double interval) {
        return m_timer->addTimer(cb, when, interval);
    }

    void EventLoop::deleteTimerEvent(TimerId timerId) {
        m_timer->deleteTimer(timerId);
    }


}

