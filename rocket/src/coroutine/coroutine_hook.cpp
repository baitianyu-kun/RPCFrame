//
// Created by baitianyu on 8/30/24.
//
#include "coroutine/coroutine_hook.h"

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);

HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(sleep);

namespace rocket {

    int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {

    }

    ssize_t read_hook(int fd, void *buf, size_t count) {

    }

    ssize_t write_hook(int fd, const void *buf, size_t count) {

    }

    int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {

    }

    unsigned int sleep_hook(unsigned int seconds) {

    }

    void SetHook(bool val) {
        g_hook = val;
    }

}