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

int main() {
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
    rocket::FDEvent event(listenfd);
    event.listen(rocket::FDEvent::IN_EVENT, [listenfd]() {
        // 如果发生in事件的话，那么应该执行该event的回调函数
        // 这里就是执行接收连接并打印client的地址
        sockaddr_in client_address;
        memset(&client_address,0, sizeof(client_address));
        socklen_t client_addr_len = sizeof(client_address);
        int client_fd = accept(listenfd, (sockaddr *) &client_address, &client_addr_len);
        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", client_fd, inet_ntoa(client_address.sin_addr),
                 ntohs(client_address.sin_port));
    });

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    auto *eventloop = new rocket::EventLoop();
    // 添加该事件
    eventloop->addEpollEvent(&event);
    // 执行loop
    eventloop->loop();

}