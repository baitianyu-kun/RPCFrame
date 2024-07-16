//
// Created by baitianyu on 7/16/24.
//

#ifndef RPCFRAME_TIMER_EVENT_H
#define RPCFRAME_TIMER_EVENT_H

#include <map>
#include "common/mutex.h"
#include "net/fd_event.h"


namespace rocket {

    class TimerEvent {
    public:
        TimerEvent(int interval, bool is_repeated, std::function<void()> callback);

        int64_t getArriveTime() const {
            return m_arrive_time;
        }

        void setCanceled(bool is_canceled) {
            m_is_canceled = is_canceled;
        }

        bool isCanceled() const {
            return m_is_canceled;
        }

        bool isRepeated() const {
            return m_is_repeated;
        }

        std::function<void()> getCallBack() {
            return m_task_callback;
        }

        // 用于计算定时事件的到达时间。假设m_interval表示一个时间间隔，
        // 例如定时器的触发间隔，通过将当前时间与间隔相加，可以得到定时事件的预期到达时间。
        void setArriveTime();

    private:
        // 单位ms，前面的m代表member表示成员
        int64_t m_arrive_time;
        // 时间间隔
        int64_t m_interval;
        bool m_is_repeated{false};
        bool m_is_canceled{false};
        std::function<void()> m_task_callback;

    };

}

#endif //RPCFRAME_TIMER_EVENT_H
