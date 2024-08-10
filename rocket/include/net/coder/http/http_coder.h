//
// Created by baitianyu on 8/9/24.
//

#ifndef RPCFRAME_HTTP_CODER_H
#define RPCFRAME_HTTP_CODER_H

#include "net/coder/abstract_coder.h"
#include "net/coder/http/http_request.h"
#include "net/coder/http/http_response.h"

namespace rocket {

    class HTTPCoder : public AbstractCoder {
    public:
        HTTPCoder() = default;

        ~HTTPCoder() override = default;

        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ out_buffer, bool is_http_client = false) override;

        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPBuffer::tcp_buffer_sptr_t_ in_buffer) override;

    private:
        bool parseHTTPRequestLine(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);

        bool parseHTTPRequestHeader(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);

        bool parseHTTPRequestContent(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);
    };

}

#endif //RPCFRAME_HTTP_CODER_H
