//
// Created by baitianyu on 7/18/24.
//
#include <unistd.h>
#include <sys/eventfd.h>
#include "event/wake_up_fd_event.h"
#include "common/log.h"

namespace mrpc {
    WakeUpFDEvent::WakeUpFDEvent() {
        m_fd = eventfd(0, EFD_NONBLOCK);
        if (m_fd < 0) {
            ERRORLOG("failed to create eventfd, error info [%d]", errno);
            exit(0);
        }
        INFOLOG("wakeup fd [%d]", m_fd);
        listen(FDEvent::IN_EVENT, std::bind(&WakeUpFDEvent::onWakeup, this));
    }

    WakeUpFDEvent::~WakeUpFDEvent() {
        close(m_fd);
        cancel_listen(FDEvent::IN_EVENT);
    }

    void WakeUpFDEvent::wakeup() {
        char buf[WAKE_UP_BUFF_LEN] = {'a'};
        int ret = write(m_fd, buf, WAKE_UP_BUFF_LEN);
        if (ret != WAKE_UP_BUFF_LEN) {
            ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
        }
    }

    void WakeUpFDEvent::onWakeup() {
        char buff[WAKE_UP_BUFF_LEN];
        // 该错误经常出现在当应用程序进行一些非阻塞(non-blocking)操作(对文件或socket)的时候。
        // 以O_NONBLOCK的标志打开文件/socket/FIFO，如果你连续做read操作而没有数据可读，此时程序不会阻塞起来等待数据准备就绪返回
        // read函数会返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。
        while (read(m_fd, buff, 8) != -1 && errno != EAGAIN) {}
        DEBUGLOG("wake up succeed! read full bytes from wakeup fd[%d]", m_fd);
    }
}