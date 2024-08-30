//
// Created by baitianyu on 8/30/24.
//

#include "net/fd_event.h"
#include "net/fd_event_pool.h"
#include "common/log.h"
#include "coroutine/coroutine.h"
#include "coroutine/coroutine_hook.h"

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);

HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(sleep);

namespace rocket {

    void toEpoll(FDEvent::fd_event_sptr_t_ fd_event, FDEvent::TriggerEventType event_type) {
        auto cur_cor = Coroutine::GetCurrentCoroutine(); // 保存当前的协程，等待eventloop中的回调函数往回resume
        if (event_type == FDEvent::TriggerEventType::IN_EVENT) {
            DEBUGLOG("cor [%d], fd [%d] register IN_EVENT to epoll", cur_cor->getCorId(), fd_event->getFD());
            // 可读事件，再从主协程中回到read协程中去直接调用系统read函数
            fd_event->listen(FDEvent::TriggerEventType::IN_EVENT,
                             [cur_cor, fd_event]() {
                                 Coroutine::Resume(cur_cor);
                             });
        } else if (event_type == FDEvent::TriggerEventType::OUT_EVENT) {
            DEBUGLOG("cor [%d], fd [%d] register OUT_EVENT to epoll", cur_cor->getCorId(), fd_event->getFD());
            fd_event->listen(FDEvent::TriggerEventType::OUT_EVENT,
                             [cur_cor, fd_event]() {
                                 Coroutine::Resume(cur_cor);
                             });
        }
        EventLoop::GetCurrentEventLoop()->addEpollEvent(fd_event);
        // toEpoll返回注册完之后，read函数调用就Yield切换到主协程去了，当yield返回时，直接调用系统 read 函数。
    }

    int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {

    }

    ssize_t read_hook(int fd, void *buf, size_t count) {
        DEBUGLOG("now in hook read");
        if (Coroutine::IsMainCoroutine()) {
            DEBUGLOG("hook disable, call sys read func"); // 在主协程的话就调用系统read函数
            return g_sys_read_fun(fd, buf, count);
        }
        auto fd_event = FDEventPool::GetFDEventPool()->getFDEvent(fd);
        fd_event->setNonBlock(); // 设置为非阻塞，否则就会阻塞在read而不是epoll wait
        // 先直接调用一次系统read函数，如果有数据返回就可以直接退出了，没必要在注册到epoll中了。
        auto n = g_sys_read_fun(fd, buf, count);
        if (n > 0) {
            return n;
        }
        toEpoll(fd_event, FDEvent::TriggerEventType::IN_EVENT);
        DEBUGLOG("hook read func to yield");
        Coroutine::Yield(); // hook之后read到这里就暂停回到主协程，然后有epoll event的话就会重新resume到这里来执行系统的read
        // 表面上来看read等待数据的时候会切到主协程中继续其他操作，然后一有数据就到这里重新执行系统的read方法
        DEBUGLOG("hook read func back, now call to sys read");
        fd_event->cancel_listen(FDEvent::TriggerEventType::IN_EVENT);
        return g_sys_read_fun(fd, buf, count);
    }

    ssize_t write_hook(int fd, const void *buf, size_t count) {
        DEBUGLOG("now in hook write");
        if (Coroutine::IsMainCoroutine()) {
            DEBUGLOG("hook disable, call sys write func");
            return g_sys_write_fun(fd, buf, count);
        }
        auto fd_event = FDEventPool::GetFDEventPool()->getFDEvent(fd);
        fd_event->setNonBlock();
        auto n = g_sys_write_fun(fd, buf, count);
        if (n > 0) {
            return n;
        }
        toEpoll(fd_event, FDEvent::TriggerEventType::OUT_EVENT);
        DEBUGLOG("hook write func to yield");
        Coroutine::Yield();
        DEBUGLOG("hook write func back, now call to sys read");
        fd_event->cancel_listen(FDEvent::TriggerEventType::OUT_EVENT);
        return g_sys_write_fun(fd, buf, count);
    }

    int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    }

    unsigned int sleep_hook(unsigned int seconds) {

    }

    void SetHook(bool val) {
        g_hook = val;
    }

}

extern "C" {
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    if (!rocket::g_hook) {
        return g_sys_accept_fun(sockfd, addr, addrlen);
    } else {
        return rocket::accept_hook(sockfd, addr, addrlen);
    }
}

ssize_t read(int fd, void *buf, size_t count) {
    if (!rocket::g_hook) {
        return g_sys_read_fun(fd, buf, count);
    } else {
        return rocket::read_hook(fd, buf, count);
    }
}

ssize_t write(int fd, const void *buf, size_t count) {
    if (!rocket::g_hook) {
        return g_sys_write_fun(fd, buf, count);
    } else {
        return rocket::write_hook(fd, buf, count);
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (!rocket::g_hook) {
        return g_sys_connect_fun(sockfd, addr, addrlen);
    } else {
        return rocket::connect_hook(sockfd, addr, addrlen);
    }
}

}

