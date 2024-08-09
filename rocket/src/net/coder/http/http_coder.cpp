//
// Created by baitianyu on 8/9/24.
//
#include <sstream>
#include "net/coder/http/http_coder.h"
#include "net/coder/http/http_define.h"
#include "common/log.h"

namespace rocket {

    void HTTPCoder::encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                           TCPBuffer::tcp_buffer_sptr_t_ out_buffer) {
        for (const auto &in_message: in_messages) {
            auto response = std::dynamic_pointer_cast<HTTPResponse>(in_message);
            std::stringstream ss;
            ss << response->m_response_version << " "
               << response->m_response_code << " "
               << response->m_response_info << g_CRLF
               << response->m_response_properties.toHTTPString() << g_CRLF
               << response->m_response_body;
            std::string http_res = ss.str();
            out_buffer->writeToBuffer(http_res.c_str(), http_res.length());
        }
    }

    void HTTPCoder::decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                           TCPBuffer::tcp_buffer_sptr_t_ in_buffer) {

    }

    bool HTTPCoder::parseHttpRequestLine(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        return false;
    }

    bool HTTPCoder::parseHttpRequestHeader(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        return false;
    }

    bool HTTPCoder::parseHttpRequestContent(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        return false;
    }

}


