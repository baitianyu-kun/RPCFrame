//
// Created by baitianyu on 8/10/24.
//
#include "net/coder/tinypb/tinypb_dispatcher.h"

namespace rocket {

    TinyPBDispatcher::~TinyPBDispatcher() {

    }

    void rocket::TinyPBDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code,
                                                  const std::string &err_info) {
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();
    }

    TinyPBDispatcher::TinyPBData TinyPBDispatcher::getBodyData(std::shared_ptr<TinyPBProtocol> &req_protocol) {
        return {req_protocol->m_method_full_name, req_protocol->m_pb_data};
    }

}



