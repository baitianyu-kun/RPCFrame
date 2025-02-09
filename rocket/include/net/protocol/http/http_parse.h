//
// Created by baitianyu on 2/8/25.
//

#ifndef RPCFRAME_HTTP_PARSE_H
#define RPCFRAME_HTTP_PARSE_H

#include <memory>
#include "net/protocol/http/http_define.h"

namespace rocket {
    class HTTPRequestParser {
    public:
        using ptr = std::shared_ptr<HTTPRequestParser>;

        HTTPRequestParser() = default;

        ~HTTPRequestParser() = default;

        bool parse(std::string &str);

        HTTPRequest::ptr getRequest() { return m_request; }

    private:
        bool parseHTTPRequestLine(const std::string &tmp);

        bool parseHTTPRequestHeader(const std::string &tmp);

        bool parseHTTPRequestContent(const std::string &tmp);

    private:
        HTTPRequest::ptr m_request;

    };

    class HTTPResponseParser {
    public:
        using ptr = std::shared_ptr<HTTPResponseParser>;

        HTTPResponseParser() = default;

        ~HTTPResponseParser() = default;

        bool parse(std::string &str);

        HTTPResponse::ptr getResponse() { return m_response; }

    private:
        bool parseHTTPResponseLine(const std::string &tmp);

        bool parseHTTPResponseHeader(const std::string &tmp);

        bool parseHTTPResponseContent(const std::string &tmp);

    private:
        HTTPResponse::ptr m_response;
    };
}

#endif //RPCFRAME_HTTP_PARSE_H
