//
// Created by baitianyu on 7/23/24.
//
#include "net/coder/tinypb/tinypb_protocol.h"

namespace rocket {
    // 静态类外进行初始化
    char TinyPBProtocol::PB_START = 0x02;
    char TinyPBProtocol::PB_END = 0x03;
}