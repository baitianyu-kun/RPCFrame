//
// Created by baitianyu on 2/15/25.
//
#include "memory"
#include <unistd.h>
#include <utility>
#include <sys/timerfd.h>
#include "event/timer_queue.h"
#include "common/log.h"

namespace mrpc {

    TimerQueue::TimerQueue() {
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        DEBUGLOG("create timer fd=%d", m_fd);
        listen(FDEvent::IN_EVENT, std::bind(&TimerQueue::handleRead, this));
    }

    TimerQueue::~TimerQueue() {
        close(m_fd);
        cancel_listen(FDEvent::IN_EVENT);
        for (const Entry &timer: timers_) {
            delete timer.second;
        }
    }

    TimerId TimerQueue::addTimer(TimerQueue::TimerCallback cb, Timestamp when, double interval) {
        Timer *timer = new Timer(std::move(cb), when, interval);
        addTimerInLoop(timer);
        return TimerId(timer, timer->sequence());
    }

    void TimerQueue::addTimerInLoop(Timer *timer) {
        // 是否取代了最早的定时触发时间
        bool eraliestChanged = insert(timer);
        // 我们需要重新设置timerfd_触发时间
        if (eraliestChanged) {
            resetTimerfd(m_fd, timer->expiration());
        }
    }

    void TimerQueue::handleRead() {
        Timestamp now = Timestamp::now();
        uint64_t read_byte;
        ssize_t readn = ::read(m_fd, &read_byte, sizeof(read_byte));
        if (readn != sizeof(read_byte)) {}
        std::vector<Entry> expired = getExpired(now);
        // 遍历到期的定时器，调用回调函数
        callingExpiredTimers_ = true;
        cancelingTimers_.clear();
        for (const Entry &it: expired) {
            it.second->run();
        }
        callingExpiredTimers_ = false;
        // 重新设置这些定时器
        reset(expired, now);
    }

    void TimerQueue::resetTimerfd(int timerfd, Timestamp expiration) {
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memset(&newValue, '\0', sizeof(newValue));
        memset(&oldValue, '\0', sizeof(oldValue));

        // 超时时间 - 现在时间
        int64_t microSecondDif = expiration.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microSecondDif < 100) {
            microSecondDif = 100;
        }

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(
                microSecondDif / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microSecondDif % Timestamp::kMicroSecondsPerSecond) * 1000);
        newValue.it_value = ts;
        // 此函数会唤醒事件循环
        if (::timerfd_settime(timerfd, 0, &newValue, &oldValue)) {

        }
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
        assert(timers_.size() == activeTimers_.size());
        std::vector<Entry> expired;
        Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
        TimerList::iterator end = timers_.lower_bound(sentry);
        assert(end == timers_.end() || now < end->first);
        std::copy(timers_.begin(), end, back_inserter(expired));
        timers_.erase(timers_.begin(), end);

        for (const Entry &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            size_t n = activeTimers_.erase(timer);
            assert(n == 1);
            (void) n;
        }
        assert(timers_.size() == activeTimers_.size());
        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
        for (const Entry &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            if (it.second->repeat()
                && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
                it.second->restart(now);
                insert(it.second);
            } else {
                // FIXME move to a free list
                delete it.second; // FIXME: no delete please
            }
            // 如果重新插入了定时器，需要继续重置timerfd
            if (!timers_.empty()) {
                resetTimerfd(m_fd, (timers_.begin()->second)->expiration());
            }
        }
    }

    bool TimerQueue::insert(Timer *timer) {
        assert(timers_.size() == activeTimers_.size());
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        TimerList::iterator it = timers_.begin();
        if (it == timers_.end() || when < it->first) {
            earliestChanged = true;
        }
        {
            std::pair<TimerList::iterator, bool> result
                    = timers_.insert(Entry(when, timer));
            assert(result.second);
            (void) result;
        }
        {
            std::pair<ActiveTimerSet::iterator, bool> result
                    = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
            assert(result.second);
            (void) result;
        }
        assert(timers_.size() == activeTimers_.size());
        return earliestChanged;
    }

    void TimerQueue::cancel(TimerId timerId) {
        assert(timers_.size() == activeTimers_.size());
        ActiveTimer timer(timerId.timer_, timerId.sequence_);
        ActiveTimerSet::iterator it = activeTimers_.find(timer);
        if (it != activeTimers_.end()) {
            size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
            assert(n == 1);
            (void) n;
//            delete it->first; // 这里删除裸指针后就不能再次添加了
            activeTimers_.erase(it);
            DEBUGLOG("==== ALREADY DELETE =====");
        } else if (callingExpiredTimers_) {
            cancelingTimers_.insert(timer);
        }
        assert(timers_.size() == activeTimers_.size());
    }

    void TimerQueue::resettimer(TimerId timerId) {
        cancel(timerId);
        auto timer = timerId.timer_;
        Timestamp newtimp(addTime(Timestamp::now(), 10)); // 重新设置定时器，根据间隔10s
        timer->setexpiration(newtimp);
        addTimerInLoop(timer);
        DEBUGLOG("===== ALREADY RESET ====");
    }
}