//
// Created by baitianyu on 2/18/25.
//

#ifndef RPCFRAME_MPB_PARSE_H
#define RPCFRAME_MPB_PARSE_H

#include "net/protocol/mpb/mpb_define.h"
#include "net/protocol/parse.h"

namespace mrpc {
    class MPbProtocolParser : public ProtocolParser {
    public:
        using ptr = std::shared_ptr<MPbProtocolParser>;

        MPbProtocolParser() = default;

        ~MPbProtocolParser() override = default;

        bool parse(std::string &str) override;
    };
}

#endif //RPCFRAME_MPB_PARSE_H
