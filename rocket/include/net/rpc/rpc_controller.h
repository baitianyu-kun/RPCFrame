//
// Created by baitianyu on 7/25/24.
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
        using rpc_controller_sptr_t_ = std::shared_ptr<RPCController>;

        RPCController() = default;

        // 析构函数虚函数写override不影响，编译器还是会先调用子类的然后调用父类的
        // 析构不加虚函数则只调用父类的
        ~RPCController() override = default;

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

        std::string GetMsgId();

        void SetLocalAddr(NetAddr::net_addr_sptr_t_ addr);

        void SetPeerAddr(NetAddr::net_addr_sptr_t_ addr);

        NetAddr::net_addr_sptr_t_ GetLocalAddr();

        NetAddr::net_addr_sptr_t_ GetPeerAddr();

        void SetTimeout(int timeout);

        int GetTimeout() const;

        bool Finished() const;

        void SetFinished(bool value);

    private:
        // 这块相当于是控制整个RPC超时啊，错误码啊，ip地址啊等参数
        int32_t m_err_code{0};
        std::string m_err_info;
        std::string m_msg_id;

        bool m_is_failed{false};
        bool m_is_canceled{false};
        bool m_is_finished{false};

        NetAddr::net_addr_sptr_t_ m_local_addr;
        NetAddr::net_addr_sptr_t_ m_peer_addr;

        int m_timeout{1000};
    };


}

#endif //RPCFRAME_RPC_CONTROLLER_H
