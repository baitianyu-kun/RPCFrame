//
// Created by baitianyu on 2/10/25.
//
#include "rpc/rpc_controller.h"
#include "common/log.h"

namespace mrpc {
    RPCController::~RPCController() {
        DEBUGLOG("~RPCController");
    }

    void mrpc::RPCController::Reset() {
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

    void RPCController::SetLocalAddr(NetAddr::ptr addr) {
        m_local_addr = addr;
    }

    void RPCController::SetPeerAddr(NetAddr::ptr addr) {
        m_peer_addr = addr;
    }

    NetAddr::ptr RPCController::GetLocalAddr() {
        return m_local_addr;
    }

    NetAddr::ptr RPCController::GetPeerAddr() {
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