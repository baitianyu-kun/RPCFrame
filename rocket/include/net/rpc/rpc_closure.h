//
// Created by baitianyu on 7/25/24.
//

#ifndef RPCFRAME_RPC_CLOSURE_H
#define RPCFRAME_RPC_CLOSURE_H

#include <functional>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

namespace rocket {
    class RPCClosure : public google::protobuf::Closure {
    public:
        RPCClosure(std::function<void()> callback) : m_callback(callback) {}

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
