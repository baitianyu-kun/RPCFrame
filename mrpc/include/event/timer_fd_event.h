//
// Created by baitianyu on 7/16/24.
//

#ifndef RPCFRAME_TIMER_FD_EVENT_H
#define RPCFRAME_TIMER_FD_EVENT_H

#include <map>
#include <memory>
#include "common/mutex.h"
#include "event/fd_event.h"


namespace mrpc {

    // 这个只是记录定时任务信息，得用timer fd event去来添加定时任务
    // 这个是单个的定时任务，下面的timer fd event是所有定时任务的集合，负责按照到达时间管理所有的定时任务
    class TimerEventInfo {
    public:
        // 给TimerEventInfo的智能指针起一个别名，多个地方都有可能会用到，所以用shared ptr
        // typedef和using差不多，只不过using可以给template的函数设置别名
        using ptr = std::shared_ptr<TimerEventInfo>;

        TimerEventInfo(int interval, bool is_repeated, std::function<void()> callback);

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
        // 比如现在是11:24:23，每隔1秒触发一次，所以下次就是11:24:24开始触发。
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

    class TimerFDEvent : public FDEvent {
    public:

        using timer_fd_event_sptr_t_ = std::shared_ptr<TimerFDEvent>;

        TimerFDEvent();

        ~TimerFDEvent();

        // 添加定时任务信息到FDEvent中
        void addTimerEvent(TimerEventInfo::ptr time_event);

        // 删除定时任务
        void deleteTimerEvent(TimerEventInfo::ptr time_event);

        // 定时时间到了之后开始执行的函数，包括从task中取出任务，删除旧的时间任务，重新放入可重复的任务，并执行相应的回调函数
        void onTimer();

    private:
        // 因为timerfd_settime设置的是间隔多少时间后进行出发，如果要到ArriveTime准时触发的话，需要当前时间now + 一个间隔interval
        // 如果time event时间大于now的时间的话，执行间隔就是其到达时间interval = ArriveTime - now，
        // 也就是隔interval时间后，就可以准确执行到这个方法。
        // 如果time event时间比现在小，是个过期的任务，加到队列的时候需要立即执行(例如设置为100ms后执行)
        void resetTimerEventArriveTime();

    private:
        // 存储当前的所有定时任务，根据到达时间进行排序
        std::multimap<int64_t, TimerEventInfo::ptr> m_pending_events;
        Mutex m_mutex; // 给pending events上锁
    };

}

#endif //RPCFRAME_TIMER_FD_EVENT_H
