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

        // is_http_client = false的情况下是这个encode，作为server去编码返回的response
        void encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                    TCPRingBuffer::tcp_ring_buff_sptr_t_ out_buffer, bool is_http_client = false) override;

        // is_http_client = true的情况下是这个encode，作为client去编码request
        void encode_request(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                            TCPRingBuffer::tcp_ring_buff_sptr_t_ out_buffer);

        // is_http_client = false的情况下是这个decode，作为server去解码client给的request
        void decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                    TCPRingBuffer::tcp_ring_buff_sptr_t_ in_buffer, bool is_http_client = false) override;

        // is_http_client = true的情况下是这个decode，作为client去解码server返回的response
        void decode_response(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                             TCPRingBuffer::tcp_ring_buff_sptr_t_ in_buffer);

    private:
        bool parseHTTPRequestLine(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);

        bool parseHTTPRequestHeader(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);

        bool parseHTTPRequestContent(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp);

        bool parseHTTPResponseLine(HTTPResponse::http_res_sptr_t_ response, const std::string &tmp);

        bool parseHTTPResponseHeader(HTTPResponse::http_res_sptr_t_ response, const std::string &tmp);

        bool parseHTTPResponseContent(HTTPResponse::http_res_sptr_t_ response, const std::string &tmp);
    };

}

#endif //RPCFRAME_HTTP_CODER_H
