//
// Created by baitianyu on 7/25/24.
//
#include "net/rpc/rpc_controller.h"
#include "common/log.h"

namespace rocket {

    RPCController::~RPCController() {
        DEBUGLOG("~RPCController");
    }

    void rocket::RPCController::Reset() {
        m_err_code = 0;
        m_err_info = "";
        m_msg_id = "";
        m_is_failed = false;
        m_is_canceled = false;
        m_local_addr = nullptr;
        m_peer_addr = nullptr;
        m_timeout = 1000; // ms
    }

    bool RPCController::Failed() const {
        return m_is_failed;
    }

    std::string RPCController::ErrorText() const {
        return m_err_info;
    }

    void RPCController::StartCancel() {
        m_is_canceled = true;
        m_is_failed = true;
    }

    void RPCController::SetFailed(const std::string &reason) {
        m_err_info = reason;
    }

    bool RPCController::IsCanceled() const {
        return m_is_canceled;
    }

    void RPCController::NotifyOnCancel(google::protobuf::Closure *callback) {

    }

    void RPCController::SetError(int32_t err_code, const std::string &err_info) {
        m_err_code = err_code;
        m_err_info = err_info;
        m_is_failed = true;
    }

    int32_t RPCController::GetErrorCode() const {
        return m_err_code;
    }

    std::string RPCController::GetErrorInfo() {
        return m_err_info;
    }

    void RPCController::SetMsgId(const std::string &msg_id) {
        m_msg_id = msg_id;
    }

    std::string RPCController::GetMSGID() const {
        return m_msg_id;
    }

    void RPCController::SetLocalAddr(NetAddr::net_addr_sptr_t_ addr) {
        m_local_addr = addr;
    }

    void RPCController::SetPeerAddr(NetAddr::net_addr_sptr_t_ addr) {
        m_peer_addr = addr;
    }

    NetAddr::net_addr_sptr_t_ RPCController::GetLocalAddr() {
        return m_local_addr;
    }

    NetAddr::net_addr_sptr_t_ RPCController::GetPeerAddr() {
        return m_peer_addr;
    }

    void RPCController::SetTimeout(int timeout) {
        m_timeout = timeout;
    }

    int RPCController::GetTimeout() const {
        return m_timeout;
    }

    bool RPCController::Finished() const {
        return m_is_finished;
    }

    void RPCController::SetFinished(bool value) {
        m_is_finished = value;
    }


}



