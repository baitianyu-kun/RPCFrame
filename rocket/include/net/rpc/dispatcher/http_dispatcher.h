//
// Created by baitianyu on 8/10/24.
//

#ifndef RPCFRAME_HTTP_DISPATCHER_H
#define RPCFRAME_HTTP_DISPATCHER_H

#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "net/rpc/abstract_dispatcher.h"

namespace rocket {

    class HTTPDispatcher : public AbstractDispatcher {
    public:
        using protobuf_service_sptr_t_ = std::shared_ptr<google::protobuf::Service>;

    public:
        HTTPDispatcher() = default;

        ~HTTPDispatcher() override;

        void dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                      const AbstractProtocol::abstract_pro_sptr_t_ &response,
                      NetAddr::net_addr_sptr_t_ peer_addr,
                      NetAddr::net_addr_sptr_t_ local_addr) override;

        void registerService(const protobuf_service_sptr_t_ &service);

        std::vector<std::string> getAllServiceName();

    private:
        static bool
        parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name);

    private:
        std::unordered_map<std::string, protobuf_service_sptr_t_> m_service_map;
    };

}

#endif //RPCFRAME_HTTP_DISPATCHER_H
