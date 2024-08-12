//
// Created by baitianyu on 8/11/24.
//

#ifndef RPCFRAME_REGISTER_DISPATCHER_H
#define RPCFRAME_REGISTER_DISPATCHER_H

#include <unordered_map>
#include <set>
#include "net/rpc/abstract_dispatcher.h"

namespace rocket {

    class RegisterDispatcher : public AbstractDispatcher {
    public:
        RegisterDispatcher() = default;

        ~RegisterDispatcher() override;

        void dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                      const AbstractProtocol::abstract_pro_sptr_t_ &response,
                      NetAddr::net_addr_sptr_t_ peer_addr,
                      NetAddr::net_addr_sptr_t_ local_addr) override;

        std::set<NetAddr::net_addr_sptr_t_> getAllServerList();

        std::string printAllMethodServer();

    private:
        // 这个应该存在dispatcher里面
        std::unordered_map<std::string, std::set<NetAddr::net_addr_sptr_t_>> m_method_server; // method对应的多少个server
    };

}

#endif //RPCFRAME_REGISTER_DISPATCHER_H
