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
        // 将message对象转换为字节流，写入到buffer
        virtual void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                            TCPBuffer::tcp_buffer_sptr_t_ out_buffer) = 0;

        // 读取buffer，并转换为message对象
        virtual void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                            TCPBuffer::tcp_buffer_sptr_t_ in_buffer) = 0;
    };

}

#endif //RPCFRAME_ABSTRACT_CODER_H
