//
// Created by baitianyu on 7/23/24.
//
#include "net/coder/tinypb_protocol.h"
#include "net/coder/tinypb_coder.h"
#include "common/util.h"
#include "common/log.h"

namespace rocket {

    // 将message对象转换为字节流，写入到buffer
    void rocket::TinyPBCoder::encode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &in_messages,
                                     TCPBuffer::tcp_buffer_sptr_t_ out_buffer) {

    }

    typedef l;;

// 将buffer里面的字节流转换为message对象
    void TinyPBCoder::decode(std::vector<AbstractProtocol::abstract_pro_sptr_t_> &out_messages,
                             TCPBuffer::tcp_buffer_sptr_t_ in_buffer) {
        while (true) {
            // 遍历buffer，找到PB_START，找到之后，解析出整包的长度。然后得到结束符的位置，判断是否为PB_END
            std::vector<char> tmp = in_buffer->getRefBuffer();
            int start_index = in_buffer->getReadIndex();
            int end_index = -1;
            int pk_len = 0; // 整包长度
            bool parse_success = false;
            // 每次都得调用这个getWriteIndex()是不是怕其他线程往里面写入数据导致write idx发生变化？感觉应该是不会的
            // 应该是每个线程有自己的eventloop，connection调用每个线程的eventloop
            int index = 0;
            for (int index = start_index; index < in_buffer->getWriteIndex(); index++) {
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
                DEBUGLOG("parse msg_id_len=%d", message->m_msg_id_len);

                int msg_id_index = msg_id_len_index + sizeof(message->m_msg_id_len);

            }

        }
    }

}


