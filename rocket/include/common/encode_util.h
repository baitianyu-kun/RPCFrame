//
// Created by baitianyu on 8/11/24.
//

#ifndef RPCFRAME_ENCODE_UTIL_H
#define RPCFRAME_ENCODE_UTIL_H

#include <cstdint>

namespace rocket {
    // 17U 表示一个无符号整数常量，具体为 17，并且在这里使用 U 后缀表示这是一个无符号整数。
    uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed = 17U);
}


#endif //RPCFRAME_ENCODE_UTIL_H
