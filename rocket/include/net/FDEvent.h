//
// Created by baitianyu on 7/15/24.
//

#ifndef RPCFRAME_FDEVENT_H
#define RPCFRAME_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

// epoll data里面是union，只能使用ptr或者是fd，所以这里可以自定义一个用户数据对象来存储数据
namespace rocket {

    class FDEvent {
    public:
        // 事件类型，使用和epoll相同
        enum TriggerEventType {
            IN_EVENT = EPOLLIN,
            OUT_EVENT = EPOLLOUT,
            ERROR_EVENT = EPOLLERR
        };

        // 防止隐式转换
        explicit FDEvent(int fd);

        FDEvent();

        ~FDEvent();

        // 设置当前event里面的fd为non-block
        // I/O多路复用 (select, epoll, kqueue, etc.)
        // ET和LT的epoll都需要将fd设置为非阻塞
        // LT通知多次，ET通知一次，如果阻塞了不就卡在那里无法继续
        // 设置新的option为nonblock，返回旧的options
        int setNonBlock();

        std::function<void()> handler(TriggerEventType event_type);

        void listen(TriggerEventType event_type, std::function<void()> callback,
                    std::function<void()> error_callback = nullptr);

        void cancel_listen(TriggerEventType event_type);

        int getFD() const {
            return m_fd;
        }

        epoll_event getEpollEvent() {
            return m_listen_event;
        }

    private:
        int m_fd{-1};

        epoll_event m_listen_event;

        // 三个回调
        std::function<void()> m_read_callback{nullptr};
        std::function<void()> m_write_callback{nullptr};
        std::function<void()> m_error_callback{nullptr};
    };

    // 这个是唤醒epoll的event，实际上就是写入8个字节，来使epoll唤醒去处理event loop里面的任务
    class WakeUpFDEvent:public FDEvent{
    public:
        explicit WakeUpFDEvent(int fd);
        ~WakeUpFDEvent();
        void wakeup();
    };

}

#endif //RPCFRAME_FDEVENT_H
