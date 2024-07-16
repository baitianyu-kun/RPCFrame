//
// Created by baitianyu on 7/16/24.
//
#include "net/timer_event.h"

#include <utility>
#include "common/util.h"

namespace rocket {

    rocket::TimerEvent::TimerEvent(int interval, bool is_repeated, std::function<void()> callback) : m_interval(
            interval), m_is_repeated(is_repeated), m_task_callback(std::move(callback)) {
        setArriveTime();
    }

    void rocket::TimerEvent::setArriveTime() {
        // 现在的ms+时间间隔
        m_arrive_time = getNowMs() + m_interval;
    }

}


