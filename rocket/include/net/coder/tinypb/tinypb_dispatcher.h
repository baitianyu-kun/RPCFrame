//
// Created by baitianyu on 8/10/24.
//

#ifndef RPCFRAME_TINYPB_DISPATCHER_H
#define RPCFRAME_TINYPB_DISPATCHER_H

#include <memory>
#include "net/coder/tinypb/tinypb_protocol.h"
#include "net/tcp/tcp_connection.h"

namespace rocket {

    class TinyPBDispatcher {
    public:

        struct TinyPBData {
            std::string m_method_full_name;
            std::string m_pb_data; // protobuf协议化数据
        };

        TinyPBDispatcher() = default;

        ~TinyPBDispatcher();

        static void setTinyPBError(std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code, const std::string &err_info);

        static TinyPBData getBodyData(std::shared_ptr<TinyPBProtocol> &req_protocol);
    };

}

#endif //RPCFRAME_TINYPB_DISPATCHER_H
