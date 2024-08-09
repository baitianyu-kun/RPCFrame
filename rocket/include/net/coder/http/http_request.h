//
// Created by baitianyu on 8/9/24.
//

#ifndef RPCFRAME_HTTP_REQUEST_H
#define RPCFRAME_HTTP_REQUEST_H

#include <memory>
#include <unordered_map>
#include "net/coder/http/http_define.h"
#include "net/coder/abstract_protocol.h"

namespace rocket {

    class HTTPRequest : public AbstractProtocol {
    public:
        using http_req_sptr_t_ = std::shared_ptr<HTTPRequest>;
    public:
        HTTPMethod m_request_method; // GET POST
        std::string m_request_path; // URL
        std::string m_request_params; // 请求参数
        std::unordered_map<std::string, std::string> m_request_params_map; // 请求参数转换为map版本
        std::string m_request_version; // http版本，例如HTTP/1.1
        HTTPHeaderProp m_request_properties; // 其他请求参数
        std::string m_request_body;
        bool parse_success{false};
    };

}

#endif //RPCFRAME_HTTP_REQUEST_H
