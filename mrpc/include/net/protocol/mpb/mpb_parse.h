//
// Created by baitianyu on 2/18/25.
//

#ifndef RPCFRAME_MPB_PARSE_H
#define RPCFRAME_MPB_PARSE_H

#include "net/protocol/mpb/mpb_define.h"

namespace mrpc {
    class MPbProtocolParser {
    public:
        using ptr = std::shared_ptr<MPbProtocolParser>;

        MPbProtocolParser() = default;

        ~MPbProtocolParser() = default;

        bool parse(std::string &str);

        MPbProtocol::ptr getRequest() { return m_protocol; }

    private:
        MPbProtocol::ptr m_protocol;
    };
}

#endif //RPCFRAME_MPB_PARSE_H
