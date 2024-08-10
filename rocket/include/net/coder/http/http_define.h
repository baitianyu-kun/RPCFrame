//
// Created by baitianyu on 8/9/24.
//

#ifndef RPCFRAME_HTTP_DEFINE_H
#define RPCFRAME_HTTP_DEFINE_H

#include <string>
#include <unordered_map>

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
        HTTP_INTERNAL_SERVER_ERROR = 500
    };

    const char* HTTPMethodToString(HTTPMethod method);

    const char *HTTPCodeToString(const int code);

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

}


#endif //RPCFRAME_HTTP_DEFINE_H
