//
// Created by baitianyu on 7/25/24.
//

#ifndef RPCFRAME_MSG_ID_UTIL_H
#define RPCFRAME_MSG_ID_UTIL_H

#include <string>

#define MAX_MSG_ID_LEN 8

namespace rocket {
    class MSGIDUtil {
    public:
        static std::string GenerateMSGID();
    };
}

#endif //RPCFRAME_MSG_ID_UTIL_H
