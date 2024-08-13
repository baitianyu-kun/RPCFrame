//
// Created by baitianyu on 8/11/24.
//

#ifndef RPCFRAME_REGISTER_DISPATCHER_H
#define RPCFRAME_REGISTER_DISPATCHER_H

#include <unordered_map>
#include <set>
#include <vector>
#include "net/rpc/abstract_dispatcher.h"
#include "net/balance/hash_balance.h"

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

        void deleteServerInServerList(NetAddr::net_addr_sptr_t_ server_addr);

        std::string printAllMethodServer();

        std::string printAllMethodBalance();

        void updateMethodServer(std::vector<std::string> method_full_name_vec, NetAddr::net_addr_sptr_t_ server_addr);

    private:
        // 这个应该存在dispatcher里面
        std::unordered_map<std::string, std::set<NetAddr::net_addr_sptr_t_, CompNetAddr>> m_method_server; // method对应的多少个server

        // 一个方法对应一个balance
        std::unordered_map<std::string, ConsistentHash::con_hash_sptr_t_> m_method_balance;
    };

}

#endif //RPCFRAME_REGISTER_DISPATCHER_H
