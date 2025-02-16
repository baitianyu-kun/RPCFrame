//
// Created by baitianyu on 2/8/25.
//

#ifndef RPCFRAME_HTTP_DEFINE_H
#define RPCFRAME_HTTP_DEFINE_H

#include <string>
#include <unordered_map>
#include <memory>
#include "net/tcp/net_addr.h"

#define RPC_METHOD_PATH "/method"
#define RPC_REGISTER_HEART_SERVER_PATH "/heart"
#define RPC_SERVER_REGISTER_PATH "/register"
#define RPC_CLIENT_REGISTER_DISCOVERY_PATH "/discovery"
#define RPC_REGISTER_SUBSCRIBE_PATH "/subscribe"
#define RPC_REGISTER_PUBLISH_PATH "/publish"

namespace mrpc {

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
        std::string m_msg_id;
        std::unordered_map<std::string, std::string> m_request_body_data_map;
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
        std::string m_msg_id;
        std::unordered_map<std::string, std::string> m_response_body_data_map;
    };

    class HTTPSession {
    public:
        using ptr = std::shared_ptr<HTTPSession>;

        HTTPSession(NetAddr::ptr local_addr,
                    NetAddr::ptr peer_addr) : m_local_addr(local_addr), m_peer_addr(peer_addr) {}

        ~HTTPSession() = default;

        NetAddr::ptr getLocalAddr() const { return m_local_addr; }

        NetAddr::ptr getPeerAddr() const { return m_peer_addr; }

    private:
        NetAddr::ptr m_local_addr;
        NetAddr::ptr m_peer_addr;
    };

    class HTTPManager {
    public:
        enum class MSGType : uint8_t {
            // 调用RPC方法后的请求与响应
            RPC_METHOD_REQUEST,
            RPC_METHOD_RESPONSE,

            // 心跳机制
            RPC_REGISTER_HEART_SERVER_REQUEST, // 注册中心向服务端发送heart pack
            RPC_REGISTER_HEART_SERVER_RESPONSE, // 服务端向注册中心回应heart pack

            // 服务注册
            RPC_SERVER_REGISTER_REQUEST, // 注册到注册中心
            RPC_SERVER_REGISTER_RESPONSE, // 注册完成后给予回应

            // 服务发现
            RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, // 客户端从注册中心中进行请求
            RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE, // 注册中心给客户端响应

            // 服务订阅
            RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST, // 客户端订阅相关service name
            RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE, // 该service name中服务器列表发生变化主动通知客户端

            // 服务推送
            RPC_REGISTER_CLIENT_PUBLISH_REQUEST, // 注册中心推送消给息客户端
            RPC_REGISTER_CLIENT_PUBLISH_RESPONSE, // 客户端收到推送
        };

        using body_type = std::unordered_map<std::string, std::string>;

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
