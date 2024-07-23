//
// Created by baitianyu on 7/23/24.
//

#ifndef RPCFRAME_TINYPB_CODER_H
#define RPCFRAME_TINYPB_CODER_H

#include <string>
#include "net/coder/abstract_coder.h"

namespace rocket {

    class TinyPBCoder : public AbstractCoder {

        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ out_buffer) override;

        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ in_buffer) override;

    };



}

#endif //RPCFRAME_TINYPB_CODER_H
