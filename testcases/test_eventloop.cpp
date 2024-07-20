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
#include "../rocket/include/common/log.h"
#include "../rocket/include/net/fd_event.h"
#include "../rocket/include/net/eventloop.h"
#include "../rocket/include/net/io_thread.h"
#include "../rocket/include/net/io_thread_pool.h"

void iothread_test() {
    int port = 22224;
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

    // 创建listen event，如果此事件发生变化的话，应该执行该event的回调函数
    rocket::FDEvent::fd_event_sptr_t_ event = std::make_shared<rocket::FDEvent>(listenfd);
    event->listen(rocket::FDEvent::IN_EVENT, [listenfd]() {
        // 如果发生in事件的话，那么应该执行该event的回调函数
        // 这里就是执行接收连接并打印client的地址
        sockaddr_in client_address;
        memset(&client_address, 0, sizeof(client_address));
        socklen_t client_addr_len = sizeof(client_address);
        int client_fd = accept(listenfd, (sockaddr *) &client_address, &client_addr_len);
        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", client_fd, inet_ntoa(client_address.sin_addr),
                 ntohs(client_address.sin_port));
    });

    // 添加定时事件
    int i = 0;
    rocket::TimerEventInfo::time_event_info_sptr_t_ time_event = std::make_shared<rocket::TimerEventInfo>(
            1000, true, [&i]() {
                INFOLOG("trigger timer event, count=%d", i++);
            }
    );

    int x = 0;
    rocket::TimerEventInfo::time_event_info_sptr_t_ time_event2 = std::make_shared<rocket::TimerEventInfo>(
            2000, true, [&x]() {
                INFOLOG("trigger timer event2, count=%d", x++);
            }
    );

    // 创建eventloop
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::IOThread io_thread;
    io_thread.getEventLoop()->addEpollEvent(event); // 添加监听事件
    io_thread.getEventLoop()->addTimerEvent(time_event); // 添加定时事件
    io_thread.start(); // 启动
    io_thread.join(); // 等待执行完成

    // 测试线程池
//    rocket::IOThreadPool io_thread_pool(2);
//    auto &io_thread1 = io_thread_pool.getIOThread();
//    io_thread1->getEventLoop()->addTimerEvent(time_event);
//    io_thread1->getEventLoop()->addEpollEvent(event);
//    auto &io_thread2 = io_thread_pool.getIOThread();
//
//    io_thread2->getEventLoop()->addTimerEvent(time_event2);
//    io_thread_pool.start();
//    io_thread_pool.join();

}

int main() {
    iothread_test();
}