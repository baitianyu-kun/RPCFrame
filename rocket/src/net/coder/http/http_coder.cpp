//
// Created by baitianyu on 8/9/24.
//
#include <sstream>
#include <algorithm>
#include "net/coder/http/http_coder.h"
#include "net/coder/http/http_define.h"
#include "common/log.h"
#include "common/string_util.h"

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
            DEBUGLOG("HTTP encode success");
        }
    }

    void HTTPCoder::decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                           TCPBuffer::tcp_buffer_sptr_t_ in_buffer) {
        while (true) {
            bool is_parse_request_line = false;
            bool is_parse_request_header = false;
            bool is_parse_request_content = false;
            int read_size = 0; // 读取长度，用来和content-length做校验
            // 转换为string开始解析
            std::string all_str(in_buffer->getRefBuffer().begin(), in_buffer->getRefBuffer().end());
            std::string tmp = all_str;
            // ================request line================
            auto i_crlf = tmp.find(g_CRLF);
            if (i_crlf == tmp.npos) {
                ERRORLOG("not found CRLF in buffer");
                continue;
            }
            if (i_crlf == tmp.length() - 2) {
                // 请求行中只有 "\r\n" 的时候不完整，需要继续读
                continue;
            }
            auto request = std::make_shared<HTTPRequest>();
            is_parse_request_line = parseHTTPRequestLine(request, tmp.substr(0, i_crlf));
            if (!is_parse_request_line) {
                return;
            }
            tmp = tmp.substr(i_crlf + 2, tmp.length() - i_crlf - 2); // 截取剩下的request properties
            read_size += i_crlf + 2;
            // ================request properties================
            // 最后一个property后面有两个\r\n
            auto i_crlf_double = tmp.find(g_CRLF_DOUBLE);
            if (i_crlf_double == tmp.npos) {
                ERRORLOG("not found last double CRLF in buffer");
                continue;
            }
            is_parse_request_header = parseHTTPRequestHeader(request, tmp.substr(0, i_crlf_double));
            tmp = tmp.substr(i_crlf_double + 4, tmp.length() - i_crlf_double - 4);
            read_size += i_crlf_double + 4;
            // ================request content================
            auto content_len = std::stoi(request->m_request_properties.m_map_properties["Content-Length"]);
            // content len 得大于上面请求行和properties的长度
            if (read_size < content_len) {
                continue;
            }
            if (request->m_request_method == HTTPMethod::POST && content_len != 0) {
                is_parse_request_content = parseHTTPRequestContent(request, tmp.substr(0, content_len));
                read_size = read_size + content_len;
            } else {
                is_parse_request_content = true; // get的时候没有请求体，请求的东西都在url上，所以直接设置为true
            }
            // ================ok================
            if (is_parse_request_line && is_parse_request_header && is_parse_request_header) {
                out_messages.emplace_back(request);
                break;
            }
        }
        DEBUGLOG("HTTP decode success");
    }

    bool HTTPCoder::parseHTTPRequestLine(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        // 请求行：请求方法，空格，URL，空格，HTTP版本，gCRLF
        auto space1 = tmp.find_first_of(" ");
        auto space2 = tmp.find_first_of(" ");
        if (space1 == tmp.npos || space2 == tmp.npos || space1 == space2) {
            ERRORLOG("parse HTTP request line error, space is not 2");
            return false;
        }
        // method
        auto method = tmp.substr(0, space1);
        std::transform(method.begin(), method.end(), method.begin(), toupper);
        if (method == "GET") {
            request->m_request_method = HTTPMethod::GET;
        } else if (method == "POST") {
            request->m_request_method = HTTPMethod::POST;
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
        request->m_request_version = version;
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
            request->m_request_path = url;
            DEBUGLOG("http request path: %s", url.c_str());
            return true;
        }
        request->m_request_path = url.substr(0, quest_mark);
        request->m_request_params = url.substr(quest_mark + 1, url.length() - quest_mark - 1);
        DEBUGLOG("http request path: %s, and params: %s", request->m_request_path.c_str(),
                 request->m_request_params.c_str());
        // id=1&age=2
        splitStrToMap(request->m_request_params, "&", "=", request->m_request_params_map);
        return true;
    }

    bool HTTPCoder::parseHTTPRequestHeader(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        if (tmp.empty() || tmp == g_CRLF_DOUBLE) {
            return true;
        }
        // Host: developer.mozilla.org \r\n
        // Content-Length: 64 \r\n
        splitStrToMap(tmp, g_CRLF, ":", request->m_request_properties.m_map_properties);
        return true;
    }

    bool HTTPCoder::parseHTTPRequestContent(HTTPRequest::http_req_sptr_t_ request, const std::string &tmp) {
        if (tmp.empty()) {
            return true;
        }
        request->m_request_body = tmp;
        return true;
    }

}


