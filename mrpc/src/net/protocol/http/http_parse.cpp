//
// Created by baitianyu on 2/8/25.
//
#include <algorithm>
#include "net/protocol/http/http_parse.h"
#include "common/log.h"
#include "common/string_util.h"

bool mrpc::HTTPRequestParser::parse(std::string &str) {
    bool is_parse_request_line = false;
    bool is_parse_request_header = false;
    bool is_parse_request_content = false;

    m_request = std::make_shared<HTTPRequest>();

    std::string tmp = str;
    // ================request line================
    auto i_crlf = tmp.find(g_CRLF);
    if (i_crlf == tmp.npos) {
        ERRORLOG("not found CRLF in buffer");
        return false;
    }
    is_parse_request_line = parseHTTPRequestLine(tmp.substr(0, i_crlf));
    if (!is_parse_request_line) {
        return false;
    }
    tmp = tmp.substr(i_crlf + 2, tmp.length() - i_crlf - 2); // 截取剩下的request properties
    // ================request properties================
    // 最后一个property后面有两个\r\n
    auto i_crlf_double = tmp.find(g_CRLF_DOUBLE);
    if (i_crlf_double == tmp.npos) {
        ERRORLOG("not found last double CRLF in buffer");
        return false;
    }
    is_parse_request_header = parseHTTPRequestHeader(tmp.substr(0, i_crlf_double));
    if (!is_parse_request_line) {
        return false;
    }
    tmp = tmp.substr(i_crlf_double + 4, tmp.length() - i_crlf_double - 4);
    // ================request content================
    int content_len = 0;
    if (m_request->m_request_properties.m_map_properties.find("Content-Length") !=
        m_request->m_request_properties.m_map_properties.end()) {
        content_len = std::stoi(m_request->m_request_properties.m_map_properties["Content-Length"]);
    }

    if (m_request->m_request_method == HTTPMethod::POST && content_len != 0) {
        is_parse_request_content = parseHTTPRequestContent(tmp.substr(0, content_len));
    } else {
        is_parse_request_content = true; // get的时候没有请求体，请求的东西都在url上，所以直接设置为true
    }
    if (!is_parse_request_content) {
        return false;
    }

    splitStrToMap(m_request->m_request_body, g_CRLF, ":", m_request->m_request_body_data_map);
    m_request->m_msg_id = m_request->m_request_body_data_map["msg_id"];

    DEBUGLOG("HTTP request decode success");
    return true;
}

bool mrpc::HTTPRequestParser::parseHTTPRequestLine(const std::string &tmp) {
    DEBUGLOG("request str: %s", tmp.c_str());
    // 请求行：请求方法，空格，URL，空格，HTTP版本，gCRLF
    auto space1 = tmp.find_first_of(" ");
    auto space2 = tmp.find_last_of(" ");
    if (space1 == tmp.npos || space2 == tmp.npos || space1 == space2) {
        ERRORLOG("parse HTTP request line error, space is not 2");
        return false;
    }
    // method
    auto method = tmp.substr(0, space1);
    std::transform(method.begin(), method.end(), method.begin(), toupper);
    if (method == "GET") {
        m_request->m_request_method = HTTPMethod::GET;
    } else if (method == "POST") {
        m_request->m_request_method = HTTPMethod::POST;
    } else {
        ERRORLOG("parse HTTP request line error, unknown method name %s, should be GET or POST", method.c_str());
        return false;
    }
    // version
    auto version = tmp.substr(space2 + 1, tmp.length() - space2 - 1);
    std::transform(version.begin(), version.end(), version.begin(), toupper);
    if (version != "HTTP/1.1" && version != "HTTP/1.0") {
        ERRORLOG("parse HTTP request line error, not support http version: %s", version.c_str());
        return false;
    }
    m_request->m_request_version = version;
    // url
    auto url = tmp.substr(space1 + 1, space2 - space1 - 1);
    auto double_slash = url.find("://");
    if (double_slash != url.npos && double_slash + 3 >= url.length()) {
        // ://只有这两个斜杠但是没有后面后续的path
        ERRORLOG("parse HTTP request line error, bad url: %s", url.c_str());
        return false;
    }
    if (double_slash == url.npos) {
        // POST /contact_form.php HTTP/1.1
        DEBUGLOG("url only have path, url: %s", url.c_str());
    } else {
        // POST http://contact_form.php HTTP/1.1，需要把http://这个前缀删除
        url = url.substr(double_slash + 3, space2 - space1 - 1 - double_slash - 3);
        DEBUGLOG("delete http prefix, url: %s", url.c_str());
        auto slash = url.find_first_of("/");
        if (slash == url.npos || slash == url.length() - 1) {
            // 请求地址为/
            DEBUGLOG("http request root path, and params is empty");
            return true;
        }
        // /contact_form.php -> contact_form.php
        url = url.substr(slash + 1, url.length() - slash - 1);
    }
    // params, contact_form.php?id=1
    auto quest_mark = url.find_first_of("?");
    if (quest_mark == url.npos) {
        m_request->m_request_path = url;
        DEBUGLOG("http request path: %s", url.c_str());
        return true;
    }
    m_request->m_request_path = url.substr(0, quest_mark);
    m_request->m_request_params = url.substr(quest_mark + 1, url.length() - quest_mark - 1);
    DEBUGLOG("http request path: %s, and params: %s", m_request->m_request_path.c_str(),
             m_request->m_request_params.c_str());
    // id=1&age=2
    splitStrToMap(m_request->m_request_params, "&", "=", m_request->m_request_params_map);
    return true;
}

bool mrpc::HTTPRequestParser::parseHTTPRequestHeader(const std::string &tmp) {
    if (tmp.empty() || tmp == g_CRLF_DOUBLE) {
        return true;
    }
    // Host: developer.mozilla.org \r\n
    // Content-Length: 64 \r\n
    splitStrToMap(tmp, g_CRLF, ":", m_request->m_request_properties.m_map_properties);
    return true;
}

bool mrpc::HTTPRequestParser::parseHTTPRequestContent(const std::string &tmp) {
    m_request->m_request_body = tmp;
    return true;
}

bool mrpc::HTTPResponseParser::parse(std::string &str) {
    // while (true) {
    bool is_parse_response_line = false;
    bool is_parse_response_header = false;
    bool is_parse_response_content = false;

    m_response = std::make_shared<HTTPResponse>();

    std::string tmp = str;
    // ================request line================
    auto i_crlf = tmp.find(g_CRLF);
    if (i_crlf == tmp.npos) {
        ERRORLOG("not found CRLF in buffer");
        return false;
    }
    is_parse_response_line = parseHTTPResponseLine(tmp.substr(0, i_crlf));
    if (!is_parse_response_line) {
        return false;
    }
    tmp = tmp.substr(i_crlf + 2, tmp.length() - i_crlf - 2); // 截取剩下的request properties
    // ================request properties================
    // 最后一个property后面有两个\r\n
    auto i_crlf_double = tmp.find(g_CRLF_DOUBLE);
    if (i_crlf_double == tmp.npos) {
        ERRORLOG("not found last double CRLF in buffer");
        return false;
    }
    is_parse_response_header = parseHTTPResponseHeader(tmp.substr(0, i_crlf_double));
    if (!is_parse_response_header) {
        return false;
    }
    tmp = tmp.substr(i_crlf_double + 4, tmp.length() - i_crlf_double - 4);
    // ================request content================
    int content_len = 0;
    if (m_response->m_response_properties.m_map_properties.find("Content-Length") !=
        m_response->m_response_properties.m_map_properties.end()) {
        content_len = std::stoi(m_response->m_response_properties.m_map_properties["Content-Length"]);
    }
    is_parse_response_content = parseHTTPResponseContent(tmp.substr(0, content_len));
    if (!is_parse_response_content) {
        return false;
    }

    splitStrToMap(m_response->m_response_body, g_CRLF, ":", m_response->m_response_body_data_map);
    m_response->m_msg_id = m_response->m_response_body_data_map["msg_id"];

    DEBUGLOG("HTTP response decode success");
    return true;
}

bool mrpc::HTTPResponseParser::parseHTTPResponseLine(const std::string &tmp) {
    DEBUGLOG("response str: %s", tmp.c_str());
    // 响应行：HTTP版本，空格，状态码，空格，状态码描述，gCRLF
    auto space1 = tmp.find_first_of(" ");
    auto space2 = tmp.find_last_of(" ");
    if (space1 == tmp.npos || space2 == tmp.npos || space1 == space2) {
        ERRORLOG("parse HTTP response line error, space is not 2");
        return false;
    }
    // version
    auto version = tmp.substr(0, space1);
    std::transform(version.begin(), version.end(), version.begin(), toupper);
    if (version != "HTTP/1.1" && version != "HTTP/1.0") {
        ERRORLOG("parse HTTP request line error, not support http version: %s", version.c_str());
        return false;
    }
    m_response->m_response_version = version;
    // http code
    auto code = tmp.substr(space1 + 1, space2 - space1 - 1);
    auto http_code = stringToHTTPCode(code);
    if (http_code == HTTPCode::HTTP_UNKNOWN_ERROR) {
        ERRORLOG("parse HTTP request line error, HTTPCode::HTTP_UNKNOWN_ERROR");
        return false;
    }
    m_response->m_response_code = http_code;
    // http code info
    auto code_info = tmp.substr(space2 + 1, tmp.length() - space2 - 1);
    m_response->m_response_info = code_info;
    return true;
}

bool mrpc::HTTPResponseParser::parseHTTPResponseHeader(const std::string &tmp) {
    if (tmp.empty() || tmp == g_CRLF_DOUBLE) {
        return true;
    }
    // Host: developer.mozilla.org\r\n
    // Content-Length: 64\r\n
    splitStrToMap(tmp, g_CRLF, ":", m_response->m_response_properties.m_map_properties);
    return true;
}

bool mrpc::HTTPResponseParser::parseHTTPResponseContent(const std::string &tmp) {
    m_response->m_response_body = tmp;
    return true;
}
