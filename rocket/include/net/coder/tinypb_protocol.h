//
// Created by baitianyu on 7/23/24.
//

#ifndef RPCFRAME_TINYPB_PROTOCOL_H
#define RPCFRAME_TINYPB_PROTOCOL_H

#include <string>
#include "net/coder/abstract_protocol.h"

namespace rocket {
    struct TinyPBProtocol : public AbstractProtocol {
    public:
        static char PB_START; // pb协议开始处
        static char PB_END;

    public:
        int32_t m_pk_len{0}; // 整包长度
        int32_t m_msg_id_len{0}; // msg id的长度
        int32_t m_method_len{0}; // 方法长度
        std::string m_method_name;
        int32_t m_err_info_len{0};
        int32_t m_err_code{0};
        std::string m_err_info;
        std::string m_pb_data; // protobuf协议化数据
        int32_t m_check_sum{0}; // 校验和
        bool parse_success{false};
    };
}

#endif //RPCFRAME_TINYPB_PROTOCOL_H
