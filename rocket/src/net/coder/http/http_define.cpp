//
// Created by baitianyu on 8/9/24.
//
#include <sstream>
#include "net/coder/http/http_define.h"

namespace rocket {

    std::string g_CRLF = "\r\n";
    std::string g_CRLF_DOUBLE = "\r\n\r\n";

    std::string content_type_text = "text/html;charset=utf-8";
    const char *default_html_template = "<html><body><h1>hello</h1><p>rpc</p></body></html>";

    const char *HTTPCodeToString(const int code) {
        switch (code) {
            case HTTP_OK:
                return "OK";
            case HTTP_BAD_REQUEST:
                return "Bad Request";
            case HTTP_FORBIDDEN:
                return "Forbidden";
            case HTTP_NOTFOUND:
                return "Not Found";
            case HTTP_INTERNAL_SERVER_ERROR:
                return "Internal Server Error";
            default:
                return "UnKnown code";
        }
    }

    HTTPCode StringToHTTPCode(std::string &code) {
        if (code == "200") {
            return HTTP_OK;
        } else if (code == "400") {
            return HTTP_BAD_REQUEST;
        } else if (code == "403") {
            return HTTP_FORBIDDEN;
        } else if (code == "404") {
            return HTTP_NOTFOUND;
        } else if (code == "500") {
            return HTTP_INTERNAL_SERVER_ERROR;
        } else {
            return HTTP_UNKNOWN_ERROR;
        }
    }

    const char *HTTPMethodToString(HTTPMethod method) {
        switch (method) {
            case HTTPMethod::GET:
                return "GET";
            case HTTPMethod::POST:
                return "POST";
            default:
                return "Unknown HTTP Method";
        }
    }

    void rocket::HTTPHeaderProp::setKeyValue(const std::string &key, const std::string &value) {
        m_map_properties[key] = value;
    }

    std::string HTTPHeaderProp::getValue(const std::string &key) {
        return m_map_properties.at(key);
    }

    std::string HTTPHeaderProp::toHTTPString() {
        std::stringstream ss;
        for (const auto &item: m_map_properties) {
            ss << item.first << ":" << item.second << g_CRLF;
        }
        return ss.str();
    }
}



