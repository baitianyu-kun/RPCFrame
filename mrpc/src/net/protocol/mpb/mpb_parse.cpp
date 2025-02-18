//
// Created by baitianyu on 2/18/25.
//
#include <iostream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include "net/protocol/mpb/mpb_parse.h"
#include "common/string_util.h"

namespace mrpc {

    bool MPbProtocolParser::parse(std::string &str) {
        m_protocol = std::make_shared<MPbProtocol>();
        const char *data = str.data();
        size_t offset = 0;
        // 检查字符串长度是否足够解析最小的头部信息
        if (str.length() < sizeof(m_protocol->m_magic) + sizeof(m_protocol->m_type) + 2 * sizeof(uint32_t)) {
            return false; // 数据不足，无法解析
        }
        // 解析 magic
        memcpy(&m_protocol->m_magic, data + offset, sizeof(m_protocol->m_magic));
        offset += sizeof(m_protocol->m_magic);
        if (m_protocol->m_magic != MAGIC) {
            return false; // 检查魔数有效性
        }
        // 解析 type
        memcpy(&m_protocol->m_type, data + offset, sizeof(m_protocol->m_type));
        offset += sizeof(m_protocol->m_type);
        // 解析 msg_id_len (网络字节序转换为本地字节序)
        uint32_t msg_id_len_net;
        memcpy(&msg_id_len_net, data + offset, sizeof(msg_id_len_net));
        offset += sizeof(msg_id_len_net);
        uint32_t msg_id_len = ntohl(msg_id_len_net);
        // 检查剩余数据是否足够解析 msg_id
        if (str.length() < offset + msg_id_len) {
            return false; // 数据不足，无法解析
        }
        // 解析 msg_id
        m_protocol->m_msg_id.assign(data + offset, msg_id_len);
        offset += msg_id_len;
        // 解析 content_len (网络字节序转换为本地字节序)
        uint32_t content_len_net;
        memcpy(&content_len_net, data + offset, sizeof(content_len_net));
        offset += sizeof(content_len_net);
        uint32_t content_len = ntohl(content_len_net);
        // 检查剩余数据是否足够解析 content
        if (str.length() < offset + content_len) {
            return false; // 数据不足，无法解析
        }
        // 解析 content
        m_protocol->m_body.assign(data + offset, content_len);
        splitStrToMap(m_protocol->m_body, g_CRLF2, ":", m_protocol->m_body_data_map);
        offset += content_len;
        // 检查是否解析了所有数据
        if (offset != str.length()) {
            return false; // 数据长度不匹配，解析失败
        }
        return true; // 解析成功
    }
}