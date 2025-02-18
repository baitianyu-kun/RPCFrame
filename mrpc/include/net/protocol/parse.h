//
// Created by baitianyu on 2/18/25.
//

#ifndef RPCFRAME_PARSE_H
#define RPCFRAME_PARSE_H

#include "net/protocol/protocol.h"

namespace mrpc {

    class ProtocolParser {
    public:
        using ptr = std::shared_ptr<ProtocolParser>;

        ProtocolParser() = default;

        virtual ~ProtocolParser() = default;

        virtual bool parse(std::string &str) = 0;

        Protocol::ptr getProtocol() { return m_protocol; }

    protected:
        Protocol::ptr m_protocol;
    };

}

#endif //RPCFRAME_PARSE_H
