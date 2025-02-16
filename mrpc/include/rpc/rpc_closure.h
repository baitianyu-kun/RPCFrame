//
// Created by baitianyu on 2/10/25.
//

#ifndef RPCFRAME_RPC_CLOSURE_H
#define RPCFRAME_RPC_CLOSURE_H

#include <functional>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

namespace mrpc {
    class RPCClosure : public google::protobuf::Closure {
    public:
        RPCClosure(std::function<void()> callback) : m_callback(callback) {}

        ~RPCClosure() override;

        void Run() override {
            if (m_callback) {
                m_callback();
            }
        };

    private:
        std::function<void()> m_callback{nullptr};
    };
}

#endif //RPCFRAME_RPC_CLOSURE_H
