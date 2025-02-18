//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_UTIL_H
#define RPCFRAME_UTIL_H

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "common/config.h"

#define NETWORK_CARD_NAME Config::GetGlobalConfig()->m_network_card_name

namespace mrpc {

    pid_t getPid();

    pid_t getThreadId();

    int64_t getNowMs();

    std::string getLocalIP();
}

#endif //RPCFRAME_UTIL_H
