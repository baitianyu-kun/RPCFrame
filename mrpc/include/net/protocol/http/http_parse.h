//
// Created by baitianyu on 2/8/25.
//

#ifndef RPCFRAME_HTTP_PARSE_H
#define RPCFRAME_HTTP_PARSE_H

#include <memory>
#include "net/protocol/http/http_define.h"
#include "net/protocol/parse.h"

namespace mrpc {
    class HTTPRequestParser : public ProtocolParser {
    public:
        using ptr = std::shared_ptr<HTTPRequestParser>;

        HTTPRequestParser() = default;

        ~HTTPRequestParser() override = default;

        bool parse(std::string &str) override;

    private:
        bool parseHTTPRequestLine(const std::string &tmp,HTTPRequest::ptr m_request);

        bool parseHTTPRequestHeader(const std::string &tmp,HTTPRequest::ptr m_request);

        bool parseHTTPRequestContent(const std::string &tmp,HTTPRequest::ptr m_request);
    };

    class HTTPResponseParser : public ProtocolParser {
    public:
        using ptr = std::shared_ptr<HTTPResponseParser>;

        HTTPResponseParser() = default;

        ~HTTPResponseParser() override = default;

        bool parse(std::string &str)override;

    private:
        bool parseHTTPResponseLine(const std::string &tmp,HTTPResponse::ptr m_response);

        bool parseHTTPResponseHeader(const std::string &tmp,HTTPResponse::ptr m_response);

        bool parseHTTPResponseContent(const std::string &tmp,HTTPResponse::ptr m_response);
    };
}

#endif //RPCFRAME_HTTP_PARSE_H
