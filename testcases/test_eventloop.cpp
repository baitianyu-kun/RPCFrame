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

void iothread_test() {
    // 创建eventloop
    mrpc::Config::SetGlobalConfig("../conf/mrpc.xml");
    mrpc::Logger::InitGlobalLogger(0,false);

    int port = 22226;
    const char *ip = "127.0.0.1";
    int backlog = 5;

    sockaddr_in base_address;
    memset(&base_address, 0, sizeof(base_address));
    base_address.sin_port = htons(port);
    base_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &base_address.sin_addr);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd != -1);
    // 先给fd绑定到地址上
    int ret = bind(listenfd, (sockaddr *) &base_address, sizeof(base_address));
    assert(ret != -1);

    ret = listen(listenfd, backlog);
    assert(ret != -1);

    mrpc::IOThread io_thread;

    // 添加定时事件
    int i = 0;
    mrpc::TimerEventInfo::ptr time_event2 = std::make_shared<mrpc::TimerEventInfo>(
            1000, true, [&i,&io_thread,&time_event2]() {
                INFOLOG("trigger timer event, count=%d", i++);
                io_thread.getEventLoop()->resetTimerEvent(time_event2);
            }
    );

//    int x = 0;
//    mrpc::TimerEventInfo::ptr time_event2 = std::make_shared<mrpc::TimerEventInfo>(
//            2000, true, [&x]() {
//                INFOLOG("trigger timer event2, count=%d", x++);
//            }
//    );



    io_thread.getEventLoop()->addTimerEvent(time_event2); // 添加定时事件
    io_thread.start(); // 启动
    io_thread.join(); // 等待执行完成


}

int main() {
    iothread_test();
}