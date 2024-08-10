//
// Created by baitianyu on 8/10/24.
//

#ifndef RPCFRAME_ABSTRACT_DISPATCHER_H
#define RPCFRAME_ABSTRACT_DISPATCHER_H

#include <memory>
#include "net/coder/abstract_protocol.h"
#include "net/tcp/net_addr.h"

namespace rocket {

    class AbstractDispatcher {
    public:
        using abstract_disp_sptr_t = std::shared_ptr<AbstractDispatcher>;

        AbstractDispatcher() = default;

        virtual ~AbstractDispatcher() = default;

        virtual void dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                              const AbstractProtocol::abstract_pro_sptr_t_ &response,
                              NetAddr::net_addr_sptr_t_ peer_addr,
                              NetAddr::net_addr_sptr_t_ local_addr) = 0;

    };

}

#endif //RPCFRAME_ABSTRACT_DISPATCHER_H
