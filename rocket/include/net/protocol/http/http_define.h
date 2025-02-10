//
// Created by baitianyu on 2/8/25.
//

#ifndef RPCFRAME_HTTP_DEFINE_H
#define RPCFRAME_HTTP_DEFINE_H

#include <string>
#include <unordered_map>
#include <memory>

#define RPC_METHOD_PATH "/method"
#define RPC_REGISTER_UPDATE_SERVER_PATH "/update"
#define RPC_SERVER_REGISTER_PATH "/register"
#define RPC_CLIENT_REGISTER_DISCOVERY_PATH "/discovery"

namespace rocket {

    // 在这里使用cpp里面的值，方便给其他引入该头文件的使用
    extern std::string g_CRLF;
    extern std::string g_CRLF_DOUBLE;

    extern std::string content_type_text;
    extern const char *default_html_template;

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

    class HTTPRequest {
    public:
        using ptr = std::shared_ptr<HTTPRequest>;

    public:
        std::string toString();

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

    class HTTPResponse {
    public:
        using ptr = std::shared_ptr<HTTPResponse>;

    public:
        std::string toString();

    public:
        std::string m_response_version;
        int m_response_code;
        std::string m_response_info;
        HTTPHeaderProp m_response_properties;
        std::string m_response_body;
    };

    class HTTPManager {
    public:
        enum class MSGType : uint8_t {
            // 调用RPC方法后的请求与响应
            RPC_METHOD_REQUEST,
            RPC_METHOD_RESPONSE,

            // 心跳机制，定时更新，只不过没有发送心跳包
            RPC_REGISTER_UPDATE_SERVER_REQUEST, // 注册中心请求服务端更新信息
            RPC_REGISTER_UPDATE_SERVER_RESPONSE, // 服务端相应注册中心更新信息

            // 服务注册
            RPC_SERVER_REGISTER_REQUEST, // 注册到注册中心
            RPC_SERVER_REGISTER_RESPONSE, // 注册完成后给予回应

            // 服务发现
            RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, // 客户端从注册中心中进行请求
            RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE, // 注册中心给客户端响应
        };

        using body_type = std::unordered_map<std::string, std::string>;

        static HTTPRequest::ptr createRequest(MSGType type, body_type body);

        static HTTPResponse::ptr createResponse(MSGType type, body_type body);

        static HTTPRequest::ptr createEmptyRequest();

        static HTTPResponse::ptr createEmptyResponse();

    private:

        static HTTPRequest::ptr createMethodRequest(body_type body);

        static HTTPRequest::ptr createUpdateRequest(body_type body);

        static HTTPRequest::ptr createRegisterRequest(body_type body);

        static HTTPRequest::ptr createDiscoveryRequest(body_type body);

        static HTTPResponse::ptr createMethodResponse(body_type body);

        static HTTPResponse::ptr createUpdateResponse(body_type body);

        static HTTPResponse::ptr createRegisterResponse(body_type body);

        static HTTPResponse::ptr createDiscoveryResponse(body_type body);
    };

}

#endif //RPCFRAME_HTTP_DEFINE_H
