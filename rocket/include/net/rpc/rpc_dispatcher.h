//
// Created by baitianyu on 7/24/24.
//

#ifndef RPCFRAME_RPC_DISPATCHER_H
#define RPCFRAME_RPC_DISPATCHER_H

#include <map>
#include <memory>
#include "net/coder/abstract_protocol.h"
#include "net/coder/tinypb_protocol.h"
#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"

namespace rocket {

    class RPCDispatcher {
    public:

        RPCDispatcher() = default;

        ~RPCDispatcher() = default;

        static std::unique_ptr<RPCDispatcher> &GetRPCDispatcher();

        static void setTinyPBError(std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code, const std::string &err_info);

    public:
        using protobuf_service_sptr_t_ = std::shared_ptr<google::protobuf::Service>;

        void dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                      const AbstractProtocol::abstract_pro_sptr_t_ &response);

        void registerService(protobuf_service_sptr_t_ service);

    private:
        static bool
        parseServiceFullName(const std::string &full_name, std::string &service_name, std::string &method_name);

    private:
        std::map<std::string, protobuf_service_sptr_t_> m_service_map;
    };

}


#endif //RPCFRAME_RPC_DISPATCHER_H
