//
// Created by baitianyu on 7/16/24.
//
#include <utility>
#include <sys/timerfd.h>
#include <cstring>
#include "event/timer_fd_event.h"
#include "common/util.h"
#include "common/log.h"

namespace mrpc {

    mrpc::TimerEventInfo::TimerEventInfo(int interval, bool is_repeated, std::function<void()> callback) : m_interval(
            interval), m_is_repeated(is_repeated), m_task_callback(std::move(callback)) {
        setArriveTime();
    }

    void mrpc::TimerEventInfo::setArriveTime() {
        // 现在的ms+时间间隔
        m_arrive_time = getNowMs() + m_interval;
    }

    TimerFDEvent::TimerFDEvent() : FDEvent() {
        // 构造的时候创建一个linux的内置的timerfd，就跟wakeup设置一个eventfd一样
        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        DEBUGLOG("create timer fd=%d", m_fd);
        // 创建完成之后监听可读事件，并设置回调函数为ontimer，即内置的timerfd_create到达设定间隔后就会触发一个
        // 可读事件，随后epoll接收到可读事件后，就可以调用timerfd_create指定的回调函数，即onTimer来执行定时任务
        // 这个定时任务onTimer会遍历所有的回调函数然后都调用一遍

        // std::bind(&TimerFDEvent::onTimer, this)相当于:
        // callback = this.onTimer()，封装成员函数时候，需要传一个当前的this指针来代表调用当前对象的
        // callback，即当前对象的onTimer
        listen(FDEvent::IN_EVENT, std::bind(&TimerFDEvent::onTimer, this));
    }

    TimerFDEvent::~TimerFDEvent() {

    }

    void TimerFDEvent::addTimerEvent(TimerEventInfo::ptr time_event) {
        // 1. 如果pending events为空的话，说明没有任务，所以不用设置触发时间，在resetTimerEventArriveTime中会直接进行返回
        // 2. 如果来的新任务比当第一个任务(已经根据到达时间排序在multimap中)的时间还小
        // 说明是一个过期的任务，应该给他设置一个较快的间隔时间例如100ms来让他在后面快速执行
        // 这两种情况需要reset内置的timerfd的时间戳，否则就不用重新进行设置
        bool is_reset_timer_fd = false;
        // pending events需要加锁
        ScopeMutext<Mutex> lock(m_mutex);
        if (m_pending_events.empty()) {
            is_reset_timer_fd = true;
        } else {
            auto iter = m_pending_events.begin();
            if (iter->second->getArriveTime() > time_event->getArriveTime()) {
                is_reset_timer_fd = true;
            }
        }
        m_pending_events.emplace(time_event->getArriveTime(), time_event);
        lock.unlock();
        if (is_reset_timer_fd) {
            resetTimerEventArriveTime();
        }
    }

    void TimerFDEvent::deleteTimerEvent(TimerEventInfo::ptr time_event) {
        time_event->setCanceled(true);
        // 从pending events中删除
        ScopeMutext<Mutex> lock(m_mutex);
        // 在从小到大的排序数组中，
        // lower_bound( begin,end,num)：从数组的begin位置到end-1位置二分查找第一个大于或等于num的数字，找到返回该数字的地址，不存在则返回end。通过返回的地址减去起始地址begin,得到找到数字在数组中的下标。
        // upper_bound( begin,end,num)：从数组的begin位置到end-1位置二分查找第一个大于num的数字，找到返回该数字的地址，不存在则返回end。通过返回的地址减去起始地址begin,得到找到数字在数组中的下标。
        // 在从大到小的排序数组中，重载lower_bound()和upper_bound()
        // lower_bound( begin,end,num,greater<type>() ):从数组的begin位置到end-1位置二分查找第一个小于或等于num的数字，找到返回该数字的地址，不存在则返回end。通过返回的地址减去起始地址begin,得到找到数字在数组中的下标。
        // upper_bound( begin,end,num,greater<type>() ):从数组的begin位置到end-1位置二分查找第一个小于num的数字，找到返回该数字的地址，不存在则返回end。通过返回的地址减去起始地址begin,得到找到数字在数组中的下标。
        // int num[6]={1,2,4,7,15,34};
        // lower_bound(num,num+6,7(要查找的数));    //返回数组中第一个大于或等于被查数的值 => 7
        // upper_bound(num,num+6,7(要查找的数));    //返回数组中第一个大于被查数的值 => 15，这样可以快速定位key-value容器的value的位置
        auto begin = m_pending_events.lower_bound(time_event->getArriveTime());
        auto end = m_pending_events.upper_bound(time_event->getArriveTime());
        auto iter = begin;
        for (iter = begin; iter != end; ++iter) {
            if (iter->second == time_event) {
                break;
            }
        }
        if (iter != end) {
            m_pending_events.erase(iter);
        }
        lock.unlock();
        DEBUGLOG("success delete TimerEvent at arrive time %lld", time_event->getArriveTime());
    }

    void TimerFDEvent::onTimer() {
        // 处理缓冲区数据，防止下一次继续触发可读事件，例如LT模式的话就会一直触发
        char buff[WAKE_UP_BUFF_LEN];
        while ((read(m_fd, buff, WAKE_UP_BUFF_LEN)) == -1 && errno == EAGAIN) {}
        // 获取定时任务，就是定时任务的时间<=当前时间的任务
        std::vector<TimerEventInfo::ptr> tmp_events;
        std::vector<std::pair<int64_t, std::function<void()>>> tasks;
        auto now_time = getNowMs();

        ScopeMutext<Mutex> lock(m_mutex);
        auto iter = m_pending_events.begin();
        for (iter = m_pending_events.begin(); iter != m_pending_events.end(); ++iter) {
            if (iter->first <= now_time) {
                if (!iter->second->isCanceled()) {
                    tmp_events.emplace_back(iter->second);
                    tasks.emplace_back(iter->second->getArriveTime(), iter->second->getCallBack());
                }
            } else {
                break;
            }
        }
        // 因为从小到大排列的，所以满足 <= now time的都是在iter的左边
        m_pending_events.erase(m_pending_events.begin(), iter);
        lock.unlock();

        // 如果是time event是需要重复执行的，需要更新其下一次的时间并重新放进去执行
        for (const auto &tmp_event: tmp_events) {
            if (tmp_event->isRepeated()) {
                tmp_event->setArriveTime(); // 重新设置这个event的时间为m_arrive_time = getNowMs() + m_interval;现在的时间加上间隔
                addTimerEvent(tmp_event);
            }
        }

        // 为了保险重新设置时间，我感觉应该不设置也行
        resetTimerEventArriveTime();

        // 开始执行任务
        for (const auto &task: tasks) {
            if (task.second) {
                task.second(); // 执行回调函数，先判断是否为nullptr
            }
        }

    }

    void TimerFDEvent::resetTimerEventArriveTime() {
        // 1. 如果pending events为空的话，说明没有任务，所以不用设置触发时间，在resetTimerEventArriveTime中会直接进行返回
        // 2. 如果来的新任务比当第一个任务(已经根据到达时间排序在multimap中)的时间还小
        // 说明是一个过期的任务，应该给他设置一个较快的间隔时间例如100ms来让他在后面快速执行
        // 然后multimap是按照从小到大的到达时间来进行排序的，如果第一个大于now time的话，那么说明所有的都大于。所以下一次触发超时时间为第一个的到达时间减去当前时间。
        ScopeMutext<Mutex> lock(m_mutex);
        auto tmp_events = m_pending_events;
        lock.unlock();
        if (tmp_events.empty()) {
            return;
        }
        auto now_time = getNowMs();
        int64_t next_interval = 0;
        auto iter = tmp_events.begin();
        if (iter->second->getArriveTime() > now_time) {
            // 如果第一个即所有的到达时间都大于现在的时间的话，
            // 那么下一次触发的时间间隔就是到达的时间-现在的时间，就是隔多长时间需要唤醒内置的timerfd
            next_interval = iter->second->getArriveTime() - now_time;
        } else {
            // 如果第一个到达时间小于等于现在的时间的话，说明是个过期任务，需要给个快速执行的时间
            next_interval = 100;
        }

        // 重新设置timerfd
        // (s) ＝1000毫秒(ms), 1毫秒(ms)＝1000微秒(us), 1微秒(us)＝1000纳秒(ns)
        // next interval单位是ms, (next_interval%1000)取得除了秒以外的除不开的毫秒部分，再*1000000得到纳秒
        //  struct timespec {
        //      time_t tv_sec;                /* Seconds */
        //      long   tv_nsec;               /* Nanoseconds */
        //  };
        //  struct itimerspec {
        //      struct timespec it_interval;  /* Interval for periodic timer （定时间隔周期）*/
        //      struct timespec it_value;     /* Initial expiration (第一次超时时间)*/
        //  };
        timespec ts{next_interval / 1000, (next_interval % 1000) * 1000000};
        // 新的超时时间，超时时间就是隔多长时间开始触发，就是咱们上面说的那个设置新的间隔
        // 设置完后会自动启动定时器
        itimerspec value{{0, 0}, ts};
        if (timerfd_settime(m_fd, 0, &value, nullptr) != 0) {
            ERRORLOG("timerfd set time error, errno=%d, error=%s", errno, strerror(errno));
        }
    }
}


