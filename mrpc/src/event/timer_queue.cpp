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
    }

    TimerId TimerQueue::addTimer(TimerQueue::TimerCallback cb, Timestamp when, double interval) {
        auto timer = std::make_shared<Timer>(cb, when, interval);
        // 是否取代了最早的定时触发时间
        bool eraliestChanged = insert(timer);
        // 我们需要重新设置timerfd_触发时间
        if (eraliestChanged) {
            resetTimerfd(m_fd, timer->expiration());
        }
        return {timer, timer->sequence()};
    }

    void TimerQueue::handleRead() {
        Timestamp now = Timestamp::now();
        uint64_t read_byte;
        ssize_t readn = ::read(m_fd, &read_byte, sizeof(read_byte));
        if (readn != sizeof(read_byte)) {}
        std::vector<Entry> expired = getExpired(now);
        // 遍历到期的定时器，调用回调函数
        m_callingExpiredTimers = true;
        m_cancelingTimers.clear();
        for (const Entry &it: expired) {
            it.second->run();
        }
        m_callingExpiredTimers = false;
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
        if (microSecondDif < 100) { microSecondDif = 100; }

        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(
                microSecondDif / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
                (microSecondDif % Timestamp::kMicroSecondsPerSecond) * 1000);
        newValue.it_value = ts;

        if (::timerfd_settime(timerfd, 0, &newValue, &oldValue)) {}
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
        assert(m_timers.size() == m_activeTimers.size());
        std::vector<Entry> expired;
        Entry sentry(now, nullptr);
        auto end = m_timers.lower_bound(sentry);
//        assert(end == m_timers.end() || now < end->first);
        std::copy(m_timers.begin(), end, back_inserter(expired));
        m_timers.erase(m_timers.begin(), end);
        for (const Entry &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            size_t n = m_activeTimers.erase(timer);
            assert(n == 1);
        }
        assert(m_timers.size() == m_activeTimers.size());
        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
        for (const Entry &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            if (it.second->repeat()
                && m_cancelingTimers.find(timer) == m_cancelingTimers.end()) {
                it.second->restart(now);
                insert(it.second);
            }
        }
        if (!m_timers.empty()) {
            resetTimerfd(m_fd, m_timers.begin()->second->expiration());
        }
    }

    bool TimerQueue::insert(Timer::ptr timer) {
        assert(m_timers.size() == m_activeTimers.size());
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        auto it = m_timers.begin();
        if (it == m_timers.end() || when < it->first) {
            earliestChanged = true;
        }
        {
            std::pair<TimerList::iterator, bool> result
                    = m_timers.insert(Entry(when, timer));
        }
        {
            std::pair<ActiveTimerSet::iterator, bool> result
                    = m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
        }
        assert(m_timers.size() == m_activeTimers.size());
        return earliestChanged;
    }

    void TimerQueue::deleteTimer(TimerId timerId) {
        assert(m_timers.size() == m_activeTimers.size());
        ActiveTimer timer(timerId.m_timer, timerId.m_sequence);
        auto it = m_activeTimers.find(timer);
        if (it != m_activeTimers.end()) {
            size_t n = m_timers.erase(Entry(it->first->expiration(), it->first));
            assert(n == 1);
            m_activeTimers.erase(it);
        } else if (m_callingExpiredTimers) {

        }
        assert(m_timers.size() == m_activeTimers.size());
    }
}