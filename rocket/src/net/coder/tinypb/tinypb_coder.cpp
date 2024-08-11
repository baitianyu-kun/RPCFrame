//
// Created by baitianyu on 7/23/24.
//
#include <arpa/inet.h>
#include "net/coder/tinypb/tinypb_protocol.h"
#include "net/coder/tinypb/tinypb_coder.h"
#include "common/util.h"
#include "common/log.h"

namespace rocket {

    // 将message对象转换为字节流，写入到buffer
    void rocket::TinyPBCoder::encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                                     TCPBuffer::tcp_buffer_sptr_t_ out_buffer, bool is_http_client /*false*/) {
        for (const auto &in_message: in_messages) {
            auto msg = std::dynamic_pointer_cast<TinyPBProtocol>(in_message);
            int len = 0;
            auto buff = encodeTinyPB(msg, len);
            if (buff != nullptr && len != 0) {
                out_buffer->writeToBuffer(buff, len);
            }
            if (buff) {
                free((void *) buff);
                buff = nullptr;
            }
        }
    }

    // 将buffer里面的字节流转换为message对象
    void TinyPBCoder::decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                             TCPBuffer::tcp_buffer_sptr_t_ in_buffer, bool is_http_client /*false*/) {
        // 这里continue的话就是可以下次在读取tcp buffer里面的内容，因为有时候tcp是发不全的
        // while内部只是做了一个包的逻辑，需要一直监听
        while (true) {
            // 遍历buffer，找到PB_START，找到之后，解析出整包的长度。然后得到结束符的位置，判断是否为PB_END
            std::vector<char> tmp = in_buffer->getRefBuffer();
            int start_index = in_buffer->getReadIndex();
            int end_index = -1;
            int pk_len = 0; // 整包长度
            bool parse_success = false;
            // 每次都得调用这个getWriteIndex()是不是怕其他线程往里面写入数据导致write idx发生变化？感觉应该是不会的
            // 应该是每个线程有自己的eventloop，connection调用每个线程的eventloop
            // 不是上面的原因，是因为在tcp connection的时候，read完成后会move write index，所以write index的位置就是停止的位置
            int index = 0;
            for (index = start_index; index < in_buffer->getWriteIndex(); index++) {
                if (tmp[index] == TinyPBProtocol::PB_START) {
                    // 从下取四个字节，由于是网络字节序，需要转换为主机字节序，下面所有的int都需要转换为主机字节序
                    if (index + 1 < in_buffer->getWriteIndex()) {
                        // 碰到pb start之后就是整包长度，里面包含开始符和结束符一共的长度
                        pk_len = getInt32FromNetByte(&tmp[index + 1]);
                        DEBUGLOG("get pk_len = %d", pk_len);
                        // 结束符的索引
                        int j = index + pk_len - 1;
                        if (j >= in_buffer->getWriteIndex()) {
                            continue;
                        }
                        if (tmp[j] == TinyPBProtocol::PB_END) {
                            start_index = index;
                            end_index = j;
                            parse_success = true;
                            break;
                        }
                    }
                }
            }
            if (index >= in_buffer->getWriteIndex()) {
                DEBUGLOG("decode end, read all buffer data");
                return;
            }
            if (parse_success) {
                // 开始校验其他部分，先把可读往后挪刚刚读取的大小: 即i从1到5，共5个字符，所以5 - 1 + 1 = 5
                // 应该就是pk_len的长度吧，后面打印测试一下
                in_buffer->moveReadIndex(end_index - start_index + 1);
                auto message = std::make_shared<TinyPBProtocol>();
                message->m_pk_len = pk_len;
                // msg id len
                // 从开始符号开始，经过sizeof(char)也就是开始符字节数，然后经过sizeof(int32_t)
                // 或者sizeof(pk_len)，也就是pk_len整包长度
                // 所占字节数，然后index就到了msg id开头处，一个字节一个字节的处理
                int msg_id_len_index = start_index + sizeof(char) + sizeof(message->m_pk_len);
                if (msg_id_len_index >= end_index) {
                    message->parse_success = false;
                    ERRORLOG("parse error, msg_id_len_index[%d] >= end_index[%d]", msg_id_len_index, end_index);
                    continue;
                }
                message->m_msg_id_len = getInt32FromNetByte(&tmp[msg_id_len_index]);
                DEBUGLOG("parse msg_id_len = %d", message->m_msg_id_len);
                // msg id string
                int msg_id_index = msg_id_len_index + sizeof(message->m_msg_id_len);
                char msg_id[MAX_CHAR_ARRAY_LEN] = {0};
                memcpy(&msg_id[0], &tmp[msg_id_index], message->m_msg_id_len);
                message->m_msg_id = std::string(msg_id);
                DEBUGLOG("parse msg_id = %s", message->m_msg_id.c_str());
                // 方法名长度, msg id的下标加上msg id的长度，因为是char，所以每块是一个字节
                int method_name_len_index = msg_id_index + message->m_msg_id_len;
                if (method_name_len_index >= end_index) {
                    message->parse_success = false;
                    ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", method_name_len_index,
                             end_index);
                    continue;
                }
                message->m_method_len = getInt32FromNetByte(&tmp[method_name_len_index]);
                // method name string
                char method_name_index = method_name_len_index + sizeof(message->m_method_len);
                char method_name[MAX_CHAR_ARRAY_LEN] = {0};
                memcpy(&method_name[0], &tmp[method_name_index], message->m_method_len);
                message->m_method_full_name = std::string(method_name);
                DEBUGLOG("parse method_name = %s", message->m_method_full_name.c_str());
                // errcode
                int err_code_index = method_name_index + message->m_method_len;
                if (err_code_index >= end_index) {
                    message->parse_success = false;
                    ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", err_code_index, end_index);
                    continue;
                }
                message->m_err_code = getInt32FromNetByte(&tmp[err_code_index]);
                // error info len
                int err_info_len_index = err_code_index + sizeof(message->m_err_code);
                if (err_info_len_index >= end_index) {
                    message->parse_success = false;
                    ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", err_info_len_index, end_index);
                    continue;
                }
                message->m_err_info_len = getInt32FromNetByte(&tmp[err_info_len_index]);
                // error info string
                int err_info_index = err_info_len_index + sizeof(message->m_err_info_len);
                char err_info[MAX_CHAR_ARRAY_LEN] = {0};
                memcpy(&err_info[0], &tmp[err_info_index], message->m_err_info_len);
                message->m_err_info = std::string(err_info);
                DEBUGLOG("parse error_info = %s", message->m_err_info.c_str());
                // pb data
                int pb_data_index = err_info_index + message->m_err_info_len;
                // 整包长度减去方法长度、msg id长度、error info长度，
                // 减去开始符、结束符的两个字节
                // 减去int整包长度、msg id长度、方法名长度、错误码、错误信息长度、校验符共六个int类型，占用24字节
                int pb_data_len = message->m_pk_len - message->m_method_len
                                  - message->m_msg_id_len - message->m_err_info_len - 2 - 24;
                message->m_pb_data = std::string(&tmp[pb_data_index], pb_data_len);
                DEBUGLOG("parse pb_data = %s", message->m_pb_data.c_str());
                // 同时需要计算校验和参数
                message->parse_success = true;
                out_messages.emplace_back(message);
            }

        }
    }

    const char *TinyPBCoder::encodeTinyPB(const std::shared_ptr<TinyPBProtocol> &message, int &len) {
        if (message->m_msg_id.empty()) {
            message->m_msg_id = DEFAULT_MSG_ID;
        }
        DEBUGLOG("msg_id = %s", message->m_msg_id.c_str());
        int pk_len =
                2 + 24 + message->m_msg_id.length() + message->m_method_full_name.length() +
                message->m_err_info.length() +
                message->m_pb_data.length();
        DEBUGLOG("pk_len = %d", pk_len);
        // 使用malloc来预先分配内存，否则p指向的是随机的内存，*p = 'a'是不可以的
        // char a;     // 为a分配了空间
        // a = 'A';     // 正确
        // char* p;   // 为p随机分配了空间
        // p = &a;   // 正确
        // *p = 'A';  // 错误
        char *buff = (char *) malloc(pk_len);
        char *tmp = buff;
        // pb start
        *tmp = TinyPBProtocol::PB_START;
        tmp++;
        // pk len
        auto pk_len_net = htonl(pk_len);
        memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
        tmp += sizeof(pk_len_net);
        // msg id len
        auto msg_id_len = message->m_msg_id.length();
        auto msg_id_len_net = htonl(msg_id_len);
        memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
        tmp += sizeof(msg_id_len_net);
        // msg id
        if (!message->m_msg_id.empty()) {
            memcpy(tmp, &(message->m_msg_id[0]), msg_id_len);
            tmp += msg_id_len;
        }
        // method name len
        auto method_name_len = message->m_method_full_name.length();
        auto method_name_len_net = htonl(method_name_len);
        memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
        tmp += sizeof(method_name_len_net);
        // method name
        if (!message->m_method_full_name.empty()) {
            memcpy(tmp, &(message->m_method_full_name[0]), method_name_len);
            tmp += method_name_len;
        }
        // error code
        auto err_code_net = htonl(message->m_err_code);
        memcpy(tmp, &err_code_net, sizeof(err_code_net));
        tmp += sizeof(err_code_net);
        // error info len
        auto err_info_len = message->m_err_info.length();
        auto err_info_len_net = htonl(err_info_len);
        memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
        tmp += sizeof(err_info_len_net);
        // error info
        if (!message->m_err_info.empty()) {
            memcpy(tmp, &(message->m_err_info[0]), err_info_len);
            tmp += err_info_len;
        }
        // pb data
        if (!message->m_pb_data.empty()) {
            memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
            tmp += message->m_pb_data.length();
        }
        int32_t check_sum_net = htonl(1);
        memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
        tmp += sizeof(check_sum_net);
        // end
        *tmp = TinyPBProtocol::PB_END;

        message->m_pk_len = pk_len;
        message->m_msg_id_len = msg_id_len;
        message->m_method_full_name = method_name_len;
        message->m_err_info_len = err_info_len;
        message->parse_success = true;
        len = pk_len;
        DEBUGLOG("encode message[%s] success", message->m_msg_id.c_str());
        return buff;
    }

}


