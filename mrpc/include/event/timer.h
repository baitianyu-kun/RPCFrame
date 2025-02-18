//
// Created by baitianyu on 2/15/25.
//

#ifndef RPCFRAME_TIMER_H
#define RPCFRAME_TIMER_H

#include <memory>
#include <functional>
#include <atomic>
#include "common/timestamp.h"

namespace mrpc {

    class Timer {
    public:
        using TimerCallback = std::function<void()>;

        using ptr = std::shared_ptr<Timer>;

        Timer(TimerCallback cb, Timestamp when, double interval)
                : m_callback(std::move(cb)),
                  m_expiration(when),
                  m_interval(interval),
                  m_repeat(interval > 0.0),// 一次性定时器设置为0
                  m_sequence(m_s_numCreated++) {}

        ~Timer();

        int64_t sequence() const { return m_sequence; }

        void run() const { m_callback(); }

        Timestamp expiration() const { return m_expiration; }

        bool repeat() const { return m_repeat; }

        // 重启定时器(如果是非重复事件则到期时间置为0)
        void restart(Timestamp now);

    private:
        const TimerCallback m_callback;  // 定时器回调函数
        Timestamp m_expiration;          // 下一次的超时时刻
        const double m_interval;         // 超时时间间隔，如果是一次性定时器，该值为0
        const bool m_repeat;             // 是否重复(false 表示是一次性定时器)
        static std::atomic<unsigned int> m_s_numCreated;
        const int64_t m_sequence;
    };

    class TimerId {
    public:
        TimerId() : m_timer(nullptr), m_sequence(0) {}

        TimerId(Timer::ptr timer, int64_t seq) : m_timer(timer), m_sequence(seq) {}

        ~TimerId() {}

        Timer::ptr m_timer;
        int64_t m_sequence;
    };
}

#endif //RPCFRAME_TIMER_H
