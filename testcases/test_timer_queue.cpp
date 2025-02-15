//
// Created by baitianyu on 7/16/24.
//
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <memory>
#include <cassert>
#include "../mrpc/include/common/log.h"
#include "../mrpc/include/event/fd_event.h"
#include "../mrpc/include/event/eventloop.h"
#include "../mrpc/include/event/io_thread.h"
#include "../mrpc/include/event/io_thread_pool.h"
#include "common/timestamp.h"

namespace mrpc {
    void iothread_test() {
        // 创建eventloop
        mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
        mrpc::Logger::InitGlobalLogger(0, false);

        auto loop = EventLoop::GetCurrentEventLoop();

        auto cb1 = []() {
            DEBUGLOG("hello");
        };
        Timestamp timestamp1(addTime(Timestamp::now(), 5));
        auto id1 = loop->addTimerEvent2(std::move(cb1), timestamp1, 0);


        Timestamp timestamp2(addTime(Timestamp::now(), 1));
        int i = 0;
        loop->addTimerEvent2([loop, id1, &i]() {
            if (i <= 20) {
                loop->resettimer(id1); // 小于10秒时候，每隔1秒重设一下timer1，使得其一直不可以触发，大于10秒后timer正常触发
                i++;
                DEBUGLOG("resets")
            }
        }, timestamp2, 1);

        loop->loop();
    }
}

int main() {
    mrpc::iothread_test();
}

