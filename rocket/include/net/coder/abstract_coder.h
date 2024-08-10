//
// Created by baitianyu on 7/22/24.
//

#ifndef RPCFRAME_ABSTRACT_CODER_H
#define RPCFRAME_ABSTRACT_CODER_H

#include <vector>
#include "net/tcp/tcp_buffer.h"
#include "net/coder/abstract_protocol.h"

namespace rocket {

    class AbstractCoder {
    public:
        using abstract_coder_sptr_t_ = std::shared_ptr<AbstractCoder>;

        // 将message对象转换为字节流，写入到buffer
        // 是http client的话，则encode http request，反之encode http response
        virtual void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                            TCPBuffer::tcp_buffer_sptr_t_ out_buffer, bool is_http_client = false) = 0;

        // 读取buffer，并转换为message对象
        virtual void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                            TCPBuffer::tcp_buffer_sptr_t_ in_buffer, bool is_http_client = false) = 0;

        AbstractCoder() = default;

        virtual ~AbstractCoder() = default;
    };

}

#endif //RPCFRAME_ABSTRACT_CODER_H
