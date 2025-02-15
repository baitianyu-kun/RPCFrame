//
// Created by baitianyu on 2/15/25.
//

#ifndef RPCFRAME_TIMER_H
#define RPCFRAME_TIMER_H

#include <memory>
#include <functional>
#include "common/timestamp.h"

namespace mrpc {
    template<typename T>
    class AtomicIntegerT {
    public:
        AtomicIntegerT() : value_(0) {
        }

        T get() {
            // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
            return __sync_val_compare_and_swap(&value_, 0, 0);
        }

        T getAndAdd(T x) {
            // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
            return __sync_fetch_and_add(&value_, x);
        }

        T addAndGet(T x) {
            return getAndAdd(x) + x;
        }

        T incrementAndGet() {
            return addAndGet(1);
        }

        T decrementAndGet() {
            return addAndGet(-1);
        }

        void add(T x) {
            getAndAdd(x);
        }

        void increment() {
            incrementAndGet();
        }

        void decrement() {
            decrementAndGet();
        }

        T getAndSet(T newValue) {
            // in gcc >= 4.7: __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST)
            return __sync_lock_test_and_set(&value_, newValue);
        }

    private:
        volatile T value_;
    };

    typedef AtomicIntegerT<int32_t> AtomicInt32;
    typedef AtomicIntegerT<int64_t> AtomicInt64;

    class Timer;

    class TimerId {
    public:
        TimerId() : timer_(nullptr), sequence_(0) {}

        TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}

        Timer *timer_;
        int64_t sequence_;
    };

    class Timer {
    public:
        using TimerCallback = std::function<void()>;

        using ptr = std::shared_ptr<Timer>;

        Timer(TimerCallback cb, Timestamp when, double interval)
                : callback_(move(cb)),
                  expiration_(when),
                  interval_(interval),
                  repeat_(interval > 0.0),// 一次性定时器设置为0
                  sequence_(s_numCreated_.incrementAndGet()) {}

        int64_t sequence() const { return sequence_; }

        void run() const {
            callback_();
        }

        Timestamp expiration() const { return expiration_; }

        void setexpiration(Timestamp newexp) { expiration_ = newexp; };

        bool repeat() const { return repeat_; }

        // 重启定时器(如果是非重复事件则到期时间置为0)
        void restart(Timestamp now);

    private:
        const TimerCallback callback_;  // 定时器回调函数
        Timestamp expiration_;          // 下一次的超时时刻
        const double interval_;         // 超时时间间隔，如果是一次性定时器，该值为0
        const bool repeat_;             // 是否重复(false 表示是一次性定时器)
        static AtomicInt64 s_numCreated_;
        const int64_t sequence_;
    };
}

#endif //RPCFRAME_TIMER_H
