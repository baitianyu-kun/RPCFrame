#include "net/protocol/http/http_define.h"

//
// Created by baitianyu on 2/8/25.
//
#include <sstream>
#include "net/protocol/http/http_define.h"
#include "common/log.h"
#include "common/string_util.h"

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

    HTTPCode stringToHTTPCode(std::string &code) {
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

    void HTTPHeaderProp::setKeyValue(const std::string &key, const std::string &value) {
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

    std::string HTTPRequest::toString() {
        std::stringstream ss;
        ss << HTTPMethodToString(m_request_method) << " "
           << m_request_path << " "
           << m_request_version << g_CRLF
           << m_request_properties.toHTTPString() << g_CRLF
           << m_request_body;
        DEBUGLOG("HTTP request encode success");
        return ss.str();
    }

    std::string HTTPResponse::toString() {
        std::stringstream ss;
        ss << m_response_version << " "
           << m_response_code << " "
           << m_response_info << g_CRLF
           << m_response_properties.toHTTPString() << g_CRLF
           << m_response_body;
        DEBUGLOG("HTTP response encode success");
        return ss.str();
    }

    HTTPRequest::ptr HTTPManager::createRequest(HTTPManager::MSGType type, HTTPManager::body_type body) {
        switch (type) {
            case MSGType::RPC_METHOD_REQUEST:
                return createMethodRequest(body);
            case MSGType::RPC_REGISTER_UPDATE_SERVER_REQUEST:
                return createUpdateRequest(body);
            case MSGType::RPC_SERVER_REGISTER_REQUEST:
                return createRegisterRequest(body);
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST:
                return createDiscoveryRequest(body);
        }
    }

    HTTPResponse::ptr HTTPManager::createResponse(HTTPManager::MSGType type, HTTPManager::body_type body) {
        switch (type) {
            case MSGType::RPC_METHOD_RESPONSE:
                return createMethodResponse(body);
            case MSGType::RPC_REGISTER_UPDATE_SERVER_RESPONSE:
                return createUpdateResponse(body);
            case MSGType::RPC_SERVER_REGISTER_RESPONSE:
                return createRegisterResponse(body);
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE:
                return createDiscoveryResponse(body);
        }
    }

    HTTPRequest::ptr HTTPManager::createEmptyRequest() {
        return nullptr;
    }

    HTTPResponse::ptr HTTPManager::createEmptyResponse() {
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPRequest::ptr HTTPManager::createMethodRequest(HTTPManager::body_type body) {
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"] + g_CRLF
                               + "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_METHOD_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr HTTPManager::createUpdateRequest(HTTPManager::body_type body) {
        std::string body_str = "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_REGISTER_UPDATE_SERVER_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr HTTPManager::createRegisterRequest(HTTPManager::body_type body) {
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"] + g_CRLF
                               + "all_method_full_names" + body["all_method_full_names"] + g_CRLF
                               + "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_SERVER_REGISTER_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr HTTPManager::createDiscoveryRequest(HTTPManager::body_type body) {
        std::string body_str = "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_CLIENT_REGISTER_DISCOVERY_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPResponse::ptr HTTPManager::createMethodResponse(HTTPManager::body_type body) {
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr HTTPManager::createUpdateResponse(HTTPManager::body_type body) {
        std::string body_str = "add_method_count:" + body["add_method_count"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr HTTPManager::createRegisterResponse(HTTPManager::body_type body) {
        std::string body_str = "add_method_count:" + body["add_method_count"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr HTTPManager::createDiscoveryResponse(HTTPManager::body_type body) {
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"] + g_CRLF
                               + "msg_id:" + body["msg_id"];;
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }
}



