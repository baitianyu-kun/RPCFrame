//
// Created by baitianyu on 7/23/24.
//

#ifndef RPCFRAME_TINYPB_CODER_H
#define RPCFRAME_TINYPB_CODER_H

#include <string>
#include "net/coder/abstract_coder.h"
#include "net/coder/tinypb/tinypb_protocol.h"

#define MAX_CHAR_ARRAY_LEN 512
#define DEFAULT_MSG_ID "123456"

namespace rocket {

    class TinyPBCoder : public AbstractCoder {
    public:

        TinyPBCoder() = default;

        ~TinyPBCoder() override = default;

        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ out_buffer, bool is_http_client = false) override;

        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ in_buffer, bool is_http_client = false) override;

    private:

        const char *encodeTinyPB(const std::shared_ptr<TinyPBProtocol> &message, int &len);

    };


}

#endif //RPCFRAME_TINYPB_CODER_H
