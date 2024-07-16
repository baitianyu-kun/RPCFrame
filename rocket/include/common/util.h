//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_UTIL_H
#define RPCFRAME_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace rocket {

    pid_t getPid();

    pid_t getThreadId();

    int64_t getNowMs();

}

#endif //RPCFRAME_UTIL_H
