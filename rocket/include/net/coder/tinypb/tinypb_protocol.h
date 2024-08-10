//
// Created by baitianyu on 7/23/24.
//

#ifndef RPCFRAME_TINYPB_PROTOCOL_H
#define RPCFRAME_TINYPB_PROTOCOL_H

#include <string>
#include "net/coder/abstract_protocol.h"

namespace rocket {
    struct TinyPBProtocol : public AbstractProtocol {
    public:
        TinyPBProtocol() = default;

        ~TinyPBProtocol() override = default;

    public:
        static char PB_START; // pb协议开始处
        static char PB_END;

    public:
        // 基于这些参数，来确定一条数据的长度、位置，从而读取到精确的数据，避免粘包、半包的现象产生
        // 1. 粘包
        //    多个数据在一块，无法确定每个数据包之间的边界，从应用层角度看，像是粘到了一块，发送和接收方都有可能发生粘包问题
        //    这种现象就如同其名，指通信双方中的一端发送了多个数据包，但在另一端则被读取成了一个数据包，比如客户端发送123、ABC两个数据包，
        //    但服务端却收成的却是123ABC这一个数据包。造成这个问题的本质原因，在前面TCP的章节中讲过，这主要是因为TPC为了优化传输效率，
        //    将多个小包合并成一个大包发送，同时多个小包之间没有界限分割造成的。
        // 2. 半包
        //    发送的数据大于缓冲区长度，会拆包，指通信双方中的一端发送一个大的数据包，但在另一端被读取成了多个数据包，
        //    例如客户端向服务端发送了一个数据包：ABCDEFGXYZ，而服务端则读取成了ABCEFG、XYZ两个包，
        //    这两个包实际上都是一个数据包中的一部分，这个现象则被称之为半包问题（产生这种现象的原因在于：接收方的数据接收缓冲区过小导致的）。
        // 3. 从应用层出发，粘包、半包问题都是由于数据包与包之间，没有边界分割导致的，那想要解决这样的问题，发送方可以在每个数据包的尾部，
        //    自己拼接一个特殊分隔符，接收方读取到数据时，再根据对应的分隔符读取数据即可。或者使用下面的定义需要的数据的长度来实现，然后就会
        //    只读取到自己想要的长度，解决粘包问题。
        int32_t m_pk_len{0}; // 整包长度
        int32_t m_msg_id_len{0}; // msg id的长度
        int32_t m_method_len{0}; // 方法长度
        std::string m_method_full_name;
        int32_t m_err_info_len{0};
        int32_t m_err_code{0};
        std::string m_err_info;
        std::string m_pb_data; // protobuf协议化数据
        int32_t m_check_sum{0}; // 校验和
        bool parse_success{false};
    };
}

#endif //RPCFRAME_TINYPB_PROTOCOL_H
