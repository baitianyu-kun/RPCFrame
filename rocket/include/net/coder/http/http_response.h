//
// Created by baitianyu on 8/9/24.
//

#ifndef RPCFRAME_HTTP_RESPONSE_H
#define RPCFRAME_HTTP_RESPONSE_H

#include <memory>
#include "net/coder/abstract_protocol.h"
#include "net/coder/http/http_define.h"

namespace rocket {

    class HTTPResponse : public AbstractProtocol {
    public:
        using http_res_sptr_t_ = std::shared_ptr<HTTPResponse>;
    public:
        std::string m_response_version;
        int m_response_code;
        std::string m_response_info;
        HTTPHeaderProp m_response_properties;
        std::string m_response_body;
    };

}

#endif //RPCFRAME_HTTP_RESPONSE_H
