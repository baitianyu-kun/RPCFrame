//
// Created by baitianyu on 7/14/24.
//
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <cstring>
#include <arpa/inet.h>
#include "common/util.h"

namespace mrpc {

    static int g_pid = 0;

    static thread_local int g_thread_id = 0;

    pid_t getPid() {
        if (g_pid != 0) {
            return g_pid;
        }
        return getpid();
    }

    pid_t getThreadId() {
        if (g_thread_id != 0) {
            return g_thread_id;
        }
        return syscall(SYS_gettid);
    }

    int64_t getNowMs() {
        timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    int32_t getInt32FromNetByte(const char *buff) {
        int32_t ret;
        memcpy(&ret, buff, sizeof(ret));
        return ntohl(ret); // net to host long
    }


}