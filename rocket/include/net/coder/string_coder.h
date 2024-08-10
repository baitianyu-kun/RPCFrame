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
        StringProtocol() = default;

        ~StringProtocol() override = default;

        std::string info;
    };

    class StringCode : public AbstractCoder {
    public:
        StringCode() = default;

        ~StringCode() override = default;

        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ out_buffer) {
            for (const auto &message: in_messages) {
                auto msg = std::dynamic_pointer_cast<StringProtocol>(message);
                out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
                // auto msg_str = reinterpret_cast<char *>(msg.get());
                // out_buffer->writeToBuffer(msg_str, sizeof(msg_str));
            }
        }

        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ in_buffer) override {
            std::vector<char> res;
            in_buffer->readFromBuffer(res, in_buffer->readAbleSize());
            // 这种不太行，有可能收到的不完整，得详细的去校验数据
            // char *res_str = &res[0];
            // auto msg = std::shared_ptr<StringProtocol>(reinterpret_cast<StringProtocol *>(res_str));
            // out_messages.emplace_back(msg);
            std::string info(res.begin(), res.end());
            auto msg = std::make_shared<StringProtocol>();
            msg->info = info;
            msg->m_msg_id = "123456";
            out_messages.emplace_back(msg);
        }
    };

}

#endif //RPCFRAME_STRING_CODER_H
