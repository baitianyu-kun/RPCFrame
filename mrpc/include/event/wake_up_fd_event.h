//
// Created by baitianyu on 7/18/24.
//

#ifndef RPCFRAME_WAKE_UP_FD_EVENT_H
#define RPCFRAME_WAKE_UP_FD_EVENT_H

#include "event//fd_event.h"

namespace mrpc{
    // 这个是唤醒epoll的event，实际上就是写入8个字节，来使epoll唤醒去处理event loop里面的任务
    class WakeUpFDEvent : public FDEvent {
    public:
        using ptr = std::shared_ptr<WakeUpFDEvent>;

        WakeUpFDEvent();

        ~WakeUpFDEvent();

        // 执行唤醒
        void wakeup();

        // 唤醒时候需要执行的函数
        void onWakeup();
    };
}
#endif //RPCFRAME_WAKE_UP_FD_EVENT_H
