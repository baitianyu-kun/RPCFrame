//
// Created by baitianyu on 2/15/25.
//
#include "event/timer.h"

namespace mrpc {
    AtomicInt64 Timer::s_numCreated_;

    void Timer::restart(Timestamp now) {
        if (repeat_) {
            // 如果是重复定时事件，则继续添加定时事件，得到新事件到期事件
            expiration_ = addTime(now, interval_);
        } else {
            expiration_ = Timestamp();
        }
    }
}