//
// Created by baitianyu on 8/9/24.
//
#include "common/string_util.h"
#include "common/log.h"

namespace rocket {

    void splitStrToMap(const std::string &str, const std::string &split_str,
                       const std::string &joiner, std::unordered_map<std::string, std::string> &res) {
        if (str.empty() || split_str.empty() || joiner.empty()) {
            DEBUGLOG("str or split_str or joiner_str is empty");
            return;
        }
        std::string tmp = str;
        std::vector<std::string> vec;
        splitStrToVector(tmp, split_str, vec);
        for (const auto &item: vec) {
            // id=1, joiner为等于号，根据不同joiner分隔开key和value
            auto j = item.find_first_of(joiner);
            if (j != item.npos && j != 0) {
                std::string key = item.substr(0, j);
                std::string value = item.substr(j + joiner.length(), item.length() - j - joiner.length());
                res[key.c_str()] = value;
            }
        }
    }

    void splitStrToVector(const std::string &str, const std::string &split_str,
                          std::vector<std::string> &res) {
        if (str.empty() || split_str.empty()) {
            return;
        }
        std::string tmp = str;
        // 补上最后一个分隔符
        // hello,world,ni,hao -> hello,world,ni,hao,
        if (tmp.substr(tmp.length() - split_str.length(), split_str.length()) != split_str) {
            tmp += split_str;
        }
        while (true) {
            // 循环处理，substr一次之后截取新的tmp
            // hello,world,ni,hao -> hello -> new tmp: world,ni,hao
            size_t i = tmp.find_first_of(split_str);
            if (i == tmp.npos) {
                return;
            }
            int l = tmp.length();
            std::string x = tmp.substr(0, i);
            tmp = tmp.substr(i + split_str.length(), l - i - split_str.length());
            if (!x.empty()) {
                res.emplace_back(x);
            }
        }
    }

}