//
// Created by baitianyu on 7/22/24.
//

#ifndef RPCFRAME_STRING_CODER_H
#define RPCFRAME_STRING_CODER_H

#include "net/coder/abstract_protocol.h"
#include "net/coder/abstract_coder.h"

namespace rocket {

    class StringProtocol : public AbstractProtocol {
    public:
        std::string info;
    };

    class StringCode : public AbstractCoder {
        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ out_buffer) override {
            for (const auto &message: in_messages) {
                auto msg = std::dynamic_pointer_cast<StringProtocol>(message);
                out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
            }
        }

        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ in_buffer) override {
            
        }
    };

}

#endif //RPCFRAME_STRING_CODER_H
