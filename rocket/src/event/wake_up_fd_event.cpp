//
// Created by baitianyu on 7/18/24.
//
#include <unistd.h>
#include "event/wake_up_fd_event.h"
#include "common/log.h"

namespace rocket {
    WakeUpFDEvent::WakeUpFDEvent(int fd) : FDEvent(fd) {

    }

    WakeUpFDEvent::~WakeUpFDEvent() {

    }

    void WakeUpFDEvent::wakeup() {
        char buf[WAKE_UP_BUFF_LEN] = {'a'};
        int ret = write(m_fd, buf, WAKE_UP_BUFF_LEN);
        if (ret != WAKE_UP_BUFF_LEN) {
            ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
        }
    }
}