//
// Created by baitianyu on 25-2-10.
//
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common/string_util.h"
#include "common/log.h"

namespace mrpc {

    static int g_msg_id_length = MAX_MSG_ID_LEN;
    static int g_random_fd = -1;
    static thread_local std::string t_msg_id_number; // 多线程的当前每个msg的id值
    static thread_local std::string t_max_msg_id_number; // 最大的msg的id值

    std::string mrpc::MSGIDUtil::GenerateMSGID() {
        // 为空即没有生成之前，或者已经生成到最大值后
        if (t_msg_id_number.empty() || t_msg_id_number == t_max_msg_id_number) {
            if (g_random_fd == -1) {
                // /dev/random和/dev/urandom是Linux系统中提供的随机伪设备，这两个设备的任务，是提供永不为空的随机字节数据流。
                g_random_fd = open("/dev/urandom", O_RDONLY);
            }
            std::string res(g_msg_id_length, 0);
            if ((read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length) {
                ERRORLOG("read form /dev/urandom error");
                return "";
            }
            for (int i = 0; i < g_msg_id_length; ++i) {
                auto x = ((uint8_t) (res[i])) % 10; // 映射到0-9之间
                res[i] = x + '0';
                t_max_msg_id_number += "9";
            }
            t_msg_id_number = res;
        } else {
            // t_msg_id_number给这个字符串+1
            // 首先判断有没有大于9的，有的话就进1，从最后一位开始判断
            auto i = t_msg_id_number.length() - 1;
            while (t_msg_id_number[i] == '9' && i >= 0) { --i; }
            // 该位置+1，如果进1的话后面都变为0
            if (i >= 0) {
                t_msg_id_number[i] += 1;
                for (auto j = i + 1; j < t_msg_id_number.length(); j++) {
                    t_msg_id_number[j] = '0';
                }
            }
        }
        return t_msg_id_number;
    }

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