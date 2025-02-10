//
// Created by baitianyu on 8/9/24.
//

#ifndef RPCFRAME_STRING_UTIL_H
#define RPCFRAME_STRING_UTIL_H

#include <string>
#include <vector>
#include <unordered_map>

#define MAX_MSG_ID_LEN 8

namespace rocket {

    void splitStrToMap(const std::string &str, const std::string &split_str,
                       const std::string &joiner, std::unordered_map<std::string, std::string> &res);

    void splitStrToVector(const std::string &str, const std::string &split_str,
                          std::vector<std::string> &res);

    class MSGIDUtil {
    public:
        static std::string GenerateMSGID();
    };
}

#endif //RPCFRAME_STRING_UTIL_H
