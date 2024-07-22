//
// Created by baitianyu on 7/14/24.
//

#include <sys/epoll.h>
#include <cstring>
#include <utility>
#include <sys/eventfd.h>
#include "net/eventloop.h"
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
            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));\
        }\
        m_listen_fds.emplace(fd_event->getFD());\
        DEBUGLOG("add event success, fd[%d]", fd_event->getFD());\
    }while(0)

#define DELETE_FROM_EPOLL() \
    do{                \
        auto it = m_listen_fds.find(fd_event->getFD());\
        if (it == m_listen_fds.end()) {\
            return;\
        }\
        int op = EPOLL_CTL_DEL;\
        int ret = epoll_ctl(m_epoll_fd, op, fd_event->getFD(), nullptr);\
        if (ret == -1) {\
            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));\
        }\
        m_listen_fds.erase(fd_event->getFD());\
        DEBUGLOG("delete event success, fd[%d]", fd_event->getFD());\
    }while(0)

namespace rocket {

// 每个线程只能有一个event loop
// ThreadLocal用于保存某个线程共享变量：对于同一个static ThreadLocal，
// 不同线程只能从中get，set，remove自己的变量，而不会影响其他线程的变量。
    static thread_local EventLoop *t_current_event_loop = nullptr;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    rocket::EventLoop::EventLoop() {
        if (t_current_event_loop != nullptr) {
            ERRORLOG("failed to create event loop, this thread has created event loop");
            exit(0);
        }
        // 设置当前线程的id
        m_pid = getThreadId();
        // Since Linux 2.6.8, the size argument is ignored, but must be greater than zero;
        // 之前需要确定有多少个需要加入，现在新版本size被忽略，因为kernel使用了可扩展的data structure
        m_epoll_fd = epoll_create(10);

        if (m_epoll_fd == -1) {
            ERRORLOG("failed to create event loop, epoll create error, error code: [%d]", errno);
            exit(0);
        }
        // 初始化唤醒fd
        initWakeUpFDEevent();
        // 初始化定时任务
        initTimer();

        INFOLOG("succeed create event loop in thread %d", m_pid);
        t_current_event_loop = this;
    }

    rocket::EventLoop::~EventLoop() {
        // 需要处理fd的关闭和指针的释放，但是这里都是智能指针所以不用管
        close(m_epoll_fd);
    }

    void rocket::EventLoop::loop() {
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
                        DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFD());
                        // 需要删除出错的fd
                        deleteEpollEvent(std::shared_ptr<FDEvent>(fd_event));
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
    void rocket::EventLoop::wakeup() {
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }

    void EventLoop::dealWakeup() {

    }

    // 如果要停止，此时阻塞在epoll wait中，需要打破阻塞，然后while检测到stop flag，所以可以直接退出
    void rocket::EventLoop::stop() {
        m_stop_flag = true;
        wakeup();
    }

    // 初始化wakeup fd
    // eventfd是linux 2.6.22后系统提供的一个轻量级的进程间通信的系统调用，eventfd通过一个进程间共享的64位计数器完成进程间通信，
    // 这个计数器由在linux内核空间维护，用户可以通过调用write方法向内核空间写入一个64位的值，也可以调用read方法读取这个值。
    // 例如父进程创建子进程后，父进程休息一秒钟后向eventfd写入，然后子进程接收到变化之后输出

    // 64位，所以只需要写入8个byte就可以，所以在WakeUpFDEvent中wakeup写入8个byte
    void EventLoop::initWakeUpFDEevent() {
        // 先创建eventfd来执行唤醒操作
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0) {
            ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
            exit(0);
        }
        INFOLOG("wakeup fd = %d", m_wakeup_fd);
        // 创建wake up event事件，确定监听类型，并使用lambda固定回调函数
        m_wakeup_fd_event = std::make_shared<WakeUpFDEvent>(m_wakeup_fd);
        // 通过“函数体”后面的‘()’传入参数 auto x = [](int a){cout << a << endl;}(123);
        // 所以lambda表达式括号内的参数就是这么传入的，即123
        m_wakeup_fd_event->listen(FDEvent::IN_EVENT, [this]() {
            char buff[WAKE_UP_BUFF_LEN];
            // 该错误经常出现在当应用程序进行一些非阻塞(non-blocking)操作(对文件或socket)的时候。
            // 以O_NONBLOCK的标志打开文件/socket/FIFO，如果你连续做read操作而没有数据可读，此时程序不会阻塞起来等待数据准备就绪返回
            // read函数会返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。
            while (read(m_wakeup_fd, buff, 8) != -1 && errno != EAGAIN) {
            }
            DEBUGLOG("wake up succeed! read full bytes from wakeup fd[%d]", m_wakeup_fd);
        });
        // 添加到epoll event中
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

    void EventLoop::addEpollEvent(FDEvent::fd_event_sptr_t_ fd_event_s_ptr) {
        auto fd_event = fd_event_s_ptr.get();
        // 创建epoll event去监听wakeup的读取事件
        // epoll_event tmp_event;
        // tmp_event.events = EPOLLIN; // 读取事件
        // 添加到哪个epollfd里，要添加的fd是什么，要添加还是删除，要添加什么事件
        // 为了指定要添加的文件描述符以及感兴趣的事件类型，需要构造一个epoll_event结构并传递给epoll_ctl函数。
        // 在epoll_ctl函数调用期间，它会将tmp_event的内容从用户空间拷贝到内核空间，以便在内核中进行相应的操作。
        // 确保内核能够访问和使用这个事件结构体的内容，而不依赖于用户空间的内存。
        // int ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wakeup_fd, &tmp_event);
        if (isInLoopThread()) {
            ADD_OR_MODIFY_TO_EPOLL();
        } else {
            // TCP connection是主线程中的建立连接，但是此时eventloop在io thread中
            // 所以出现主线程和子线程同时访问一个eventloop的情况，所以需要进行处理

            // 当前thread不在当前loop中，是其他线程要往进添加
            // 因为epoll的操作必须在所属线程中进行，不能直接在当前线程中调用ADD_TO_EPOLL()函数
            // 通过调用addTask函数将这个lambda表达式封装成任务，并设置is_wake_up参数为true，以确保事件循环在添加任务后被唤醒。

            // 在addTask函数中，将任务（即 lambda 表达式）添加到待执行任务队列中，
            // 并在需要的时候调用wakeup函数唤醒事件循环线程。这样，在事件循环线程中会执行待执行任务队列中的任务，
            // 其中就包括了lambda表达式，从而间接地调用了ADD_TO_EPOLL()函数。
            // 通过这种方式，可以确保在不同线程中调用addEpollEvent时，将任务传递给事件循环所属的线程执行，
            // 以保证对 epoll 实例的操作在正确的线程中进行。
            auto callback = [this, fd_event]() {
                ADD_OR_MODIFY_TO_EPOLL();
            };
            addTask(callback, true);
        }
    }

    void EventLoop::deleteEpollEvent(FDEvent::fd_event_sptr_t_ fd_event_s_ptr) {
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

    // 判断是否是当前进程在loop中
    bool EventLoop::isInLoopThread() {
        return m_pid == getThreadId();
    }


    EventLoop *EventLoop::GetCurrentEventLoop() {
        // 在new的时候已经给current eventloop赋值过了，所以不需要了
        // 并且加了thread local，已经线程安全了
        return t_current_event_loop ? t_current_event_loop : new EventLoop();
    }

    bool EventLoop::LoopStopFlag() {
        return m_stop_flag;
    }

    void EventLoop::initTimer() {
        m_timer = std::make_shared<TimerFDEvent>();
        addEpollEvent(m_timer);
    }


    void EventLoop::addTimerEvent(TimerEventInfo::time_event_info_sptr_t_ time_event) {
        m_timer->addTimerEvent(std::move(time_event));
    }
}

