//
// Created by baitianyu on 2/10/25.
//

#ifndef RPCFRAME_RPC_CONTROLLER_H
#define RPCFRAME_RPC_CONTROLLER_H

#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "net/tcp/net_addr.h"

namespace rocket {
    class RPCController : public google::protobuf::RpcController {
    public:
        using ptr = std::shared_ptr<RPCController>;

        RPCController() = default;

        ~RPCController() override;

        void Reset() override;

        bool Failed() const override;

        std::string ErrorText() const override;

        void StartCancel() override;

        void SetFailed(const std::string &reason) override;

        bool IsCanceled() const override;

        void NotifyOnCancel(google::protobuf::Closure *callback) override;

        void SetError(int32_t err_code, const std::string &err_info);

        int32_t GetErrorCode() const;

        std::string GetErrorInfo();

        void SetMsgId(const std::string &msg_id);

        std::string GetMSGID() const;

        void SetLocalAddr(NetAddr::ptr addr);

        void SetPeerAddr(NetAddr::ptr addr);

        NetAddr::ptr GetLocalAddr();

        NetAddr::ptr GetPeerAddr();

        void SetTimeout(int timeout);

        int GetTimeout() const;

        bool Finished() const;

        void SetFinished(bool value);

    private:
        // 控制整个RPC超时，错误码，ip地址等参数
        int32_t m_err_code{0};
        std::string m_err_info;
        std::string m_msg_id;

        bool m_is_failed{false};
        bool m_is_canceled{false};
        bool m_is_finished{false};

        NetAddr::ptr m_local_addr;
        NetAddr::ptr m_peer_addr;

        int m_timeout{1000};
    };

}

#endif //RPCFRAME_RPC_CONTROLLER_H
