//
// Created by baitianyu on 2/8/25.
//

#ifndef RPCFRAME_HTTP_DEFINE_H
#define RPCFRAME_HTTP_DEFINE_H

#include <string>
#include <unordered_map>
#include <memory>
#include "net/tcp/net_addr.h"
#include "net/protocol/protocol.h"

namespace mrpc {

    enum HTTPMethod {
        GET = 1,
        POST = 2
    };

    enum HTTPCode {
        HTTP_OK = 200,
        HTTP_BAD_REQUEST = 400,
        HTTP_FORBIDDEN = 403,
        HTTP_NOTFOUND = 404,
        HTTP_INTERNAL_SERVER_ERROR = 500,
        HTTP_UNKNOWN_ERROR = 999
    };

    const char *HTTPMethodToString(HTTPMethod method);

    const char *HTTPCodeToString(const int code);

    HTTPCode stringToHTTPCode(std::string &code);

    // header中的请求参数，放到键值对里面，例如Content-Length: 55743等
    class HTTPHeaderProp {
    public:
        void setKeyValue(const std::string &key, const std::string &value);

        std::string getValue(const std::string &key);

        // map里面的内容输出为string
        std::string toHTTPString();

    public:
        std::unordered_map<std::string, std::string> m_map_properties;
    };

    class HTTPRequest: public Protocol {
    public:
        using ptr = std::shared_ptr<HTTPRequest>;

    public:
        std::string toString() override;

    public:
        HTTPMethod m_request_method; // GET POST
        std::string m_request_path; // URL
        std::string m_request_params; // 请求参数
        std::unordered_map<std::string, std::string> m_request_params_map; // 请求参数转换为map版本
        std::string m_request_version; // http版本，例如HTTP/1.1
        HTTPHeaderProp m_request_properties; // 其他请求参数
        std::string m_request_body;
    };

    class HTTPResponse: public Protocol{
    public:
        using ptr = std::shared_ptr<HTTPResponse>;

    public:
        std::string toString() override;

    public:
        std::string m_response_version;
        int m_response_code;
        std::string m_response_info;
        HTTPHeaderProp m_response_properties;
        std::string m_response_body;
    };

    class HTTPManager {
    public:

        static void createRequest(HTTPRequest::ptr request, MSGType type, body_type &body);

        static void createResponse(HTTPResponse::ptr response, MSGType type, body_type &body);

        static void createDefaultResponse(HTTPResponse::ptr response);

    private:

        static void createMethodRequest(HTTPRequest::ptr request, body_type &body);

        static void createHeartRequest(HTTPRequest::ptr request, body_type &body);

        static void createRegisterRequest(HTTPRequest::ptr request, body_type &body);

        static void createDiscoveryRequest(HTTPRequest::ptr request, body_type &body);

        static void createSubscribeRequest(HTTPRequest::ptr request, body_type &body);

        static void createPublishRequest(HTTPRequest::ptr request, body_type &body);

        static void createMethodResponse(HTTPResponse::ptr response, body_type &body);

        static void createHeartResponse(HTTPResponse::ptr response, body_type &body);

        static void createRegisterResponse(HTTPResponse::ptr response, body_type &body);

        static void createDiscoveryResponse(HTTPResponse::ptr response, body_type &body);

        static void createSubscribeResponse(HTTPResponse::ptr response, body_type &body);

        static void createPublishResponse(HTTPResponse::ptr response, body_type &body);
    };

}

#endif //RPCFRAME_HTTP_DEFINE_H
