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

    // 范围：由于 int32_t 是带符号的整数类型，它的范围是 -2147483648 到 2147483647，与普通的 int 类型相同。
    // 使用 int32_t 类型可以确保在不同平台上具有相同的宽度和范围，增强了代码的可移植性。
    // 当需要精确控制整数宽度和范围时，或者与其他系统进行交互时，使用 int32_t 是一个好的选择。
    int32_t getInt32FromNetByte(const char *buff);

    std::string getLocalIP();
}

#endif //RPCFRAME_UTIL_H
