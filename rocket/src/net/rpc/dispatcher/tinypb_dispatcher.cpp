//
// Created by baitianyu on 8/10/24.
//
#include "net/rpc/dispatcher/tinypb_dispatcher.h"
#include "net/rpc/rpc_controller.h"
#include "common/runtime.h"
#include "common/error_code.h"
#include "common/log.h"

namespace rocket {

    rocket::TinyPBDispatcher::~TinyPBDispatcher() {
        DEBUGLOG("~TinyPBDispatcher");
    }

    // 当函数参数声明为 const 引用时，这意味着在函数调用时可以传入常量或者非常量的值。const 修饰的参数表示函数在处理这些参数时不会修改它们的值。
    // 入参为常量时候，代表该函数不会修改它，调用的时候传入常量或者非常量都可以
    void TinyPBDispatcher::dispatch(const AbstractProtocol::abstract_pro_sptr_t_ &request,
                                    const AbstractProtocol::abstract_pro_sptr_t_ &response,
                                    NetAddr::net_addr_sptr_t_ peer_addr,
                                    NetAddr::net_addr_sptr_t_ local_addr) {
        auto req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
        auto rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
        // rpc调用method为：Order.makeOrder，前面是service名，后面是方法名
        auto method_full_name = req_protocol->m_method_full_name;
        std::string service_name;
        std::string method_name;
        // 赋予相同的id，二者的方法名肯定也一样
        rsp_protocol->m_msg_id = req_protocol->m_msg_id;
        rsp_protocol->m_method_full_name = req_protocol->m_method_full_name;
        if (!parseServiceFullName(method_full_name, service_name, method_name)) {
            setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
            return;
        }

        auto iter = m_service_map.find(service_name);
        if (iter == m_service_map.end()) {
            ERRORLOG("%s | service name [%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
            return;
        }

        auto service = (*iter).second;
        auto method = service->GetDescriptor()->FindMethodByName(method_name);
        if (method == nullptr) {
            ERRORLOG("%s | method name [%s] not found in service[%s]", req_protocol->m_msg_id.c_str(),
                     method_name.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
            return;
        }

        auto req_msg = std::shared_ptr<google::protobuf::Message>(service->GetRequestPrototype(method).New());
        // 反序列化，将 pb_data 反序列化为 req_msg
        // ParseFromString传的是引用
        if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
            ERRORLOG("%s | deserialize error", req_protocol->m_msg_id.c_str(), method_name.c_str(),
                     service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserialize error");
            return;
        }
        INFOLOG("%s | get rpc request[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());
        // rpc controller，目的就是在调用方法时候能够知道本地调用地址啊，超时时间等信息。
        auto rsp_msg = std::shared_ptr<google::protobuf::Message>(service->GetResponsePrototype(method).New());
        auto rpc_controller = std::make_shared<RPCController>();
        rpc_controller->SetLocalAddr(local_addr);
        rpc_controller->SetPeerAddr(peer_addr);
        rpc_controller->SetMsgId(req_protocol->m_msg_id);

        // 放入RunTime中使得log能够拿到msg id和addr
        RunTime::GetRunTime()->m_msg_id = req_protocol->m_msg_id;
        // // req_protocol->m_method_full_name是全名，例如Order.make_order，前面是service name，后面是method name
        RunTime::GetRunTime()->m_method_full_name = req_protocol->m_method_full_name;

        // Closure* done是回调函数
        service->CallMethod(method, rpc_controller.get(), req_msg.get(), rsp_msg.get(), nullptr);

        rsp_protocol->m_err_code = 0;
        // 将response message写入到pb data里面
        rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data));
        INFOLOG("%s | dispatch success, request[%s], response[%s]",
                req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str(),
                rsp_msg->ShortDebugString().c_str());
    }

    void TinyPBDispatcher::registerService(const TinyPBDispatcher::protobuf_service_sptr_t_ &service) {
        auto service_name = service->GetDescriptor()->full_name();
        m_service_map[service_name] = service;
    }

    bool TinyPBDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name,
                                                std::string &method_name) {
        if (full_name.empty()) {
            ERRORLOG("full name empty");
            return false;
        }
        // Order.makeOrder
        auto i = full_name.find_first_of(".");
        // find函数在找不到指定值得情况下会返回string::npos
        if (i == full_name.npos) {
            ERRORLOG("not find '.' in full name [%s]", full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0, i);
        method_name = full_name.substr(i + 1, full_name.length() - i - 1);
        INFOLOG("parse service_name [%s] and method_name [%s] from full name [%s]", service_name.c_str(),
                method_name.c_str(), full_name.c_str());
        return true;
    }

    void TinyPBDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code,
                                          const std::string &err_info) {
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();
    }
}

