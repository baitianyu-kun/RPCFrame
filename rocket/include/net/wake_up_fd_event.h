//
// Created by baitianyu on 7/18/24.
//

#ifndef RPCFRAME_WAKE_UP_FD_EVENT_H
#define RPCFRAME_WAKE_UP_FD_EVENT_H

#include "net/fd_event.h"

namespace rocket{
    // 这个是唤醒epoll的event，实际上就是写入8个字节，来使epoll唤醒去处理event loop里面的任务
    class WakeUpFDEvent : public FDEvent {
    public:

        explicit WakeUpFDEvent(int fd);

        ~WakeUpFDEvent();

        // 唤醒时候需要执行的函数
        void wakeup();
    };
}
#endif //RPCFRAME_WAKE_UP_FD_EVENT_H
