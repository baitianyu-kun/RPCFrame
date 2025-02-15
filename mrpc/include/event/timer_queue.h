//
// Created by baitianyu on 2/15/25.
//

#ifndef RPCFRAME_TIMER_QUEUE_H
#define RPCFRAME_TIMER_QUEUE_H

#include <vector>
#include <set>
#include "event/timer.h"
#include "event/fd_event.h"

namespace mrpc {

    class TimerQueue : public FDEvent {
    public:
        using ptr = std::shared_ptr<TimerQueue>;
        using TimerCallback = std::function<void()>;
        using Entry = std::pair<Timestamp, Timer *>; // 以时间戳作为键值获取定时器
        using TimerList = std::set<Entry>;          // 底层使用红黑树管理，自动按照时间戳进行排序
        typedef std::pair<Timer *, int64_t> ActiveTimer;
        typedef std::set<ActiveTimer> ActiveTimerSet;

        TimerQueue();

        ~TimerQueue();

        // 插入定时器（回调函数，到期时间，是否重复）
        TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

        void cancel(TimerId timerId);

        void resettimer(TimerId timerId);

    private:

        void addTimerInLoop(Timer *timer);

        // 定时器读事件触发的函数
        void handleRead();

        // 重新设置timerfd_
        void resetTimerfd(int timerfd, Timestamp expiration);

        // 移除所有已到期的定时器
        // 1.获取到期的定时器
        // 2.重置这些定时器（销毁或者重复定时任务）
        std::vector<Entry> getExpired(Timestamp now);

        void reset(const std::vector<Entry> &expired, Timestamp now);

        // 插入定时器的内部方法
        bool insert(Timer *timer);

        TimerList timers_;          // 定时器队列（内部实现是红黑树）

        bool callingExpiredTimers_; // 标明正在获取超时定时器

        // for cancel()
        ActiveTimerSet activeTimers_;
        ActiveTimerSet cancelingTimers_;
    };
}

#endif //RPCFRAME_TIMER_QUEUE_H
