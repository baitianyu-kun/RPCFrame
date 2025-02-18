//
// Created by baitianyu on 2/8/25.
//
#include "rpc/rpc_dispatcher.h"
#include "common/string_util.h"
#include "common/log.h"

namespace mrpc {

    RPCDispatcher::ptr RPCDispatcher::t_current_rpc_dispatcher = std::make_shared<RPCDispatcher>();

    RPCDispatcher::RPCDispatcher() {
        // 把相应的处理方法注册到dispatcher servlet中
        // 使用占位符来提前占住参数，等调用时候，外部参数可以通过占位符来进行转发
        m_dispatch_servlet = std::make_shared<DispatchServlet>();
    }

    void RPCDispatcher::addServlet(const std::string &uri, Servlet::ptr slt) {
        m_dispatch_servlet->addServlet(uri, slt);
    }

    void RPCDispatcher::addServlet(const std::string &uri, CallBacksServlet::callback cb) {
        m_dispatch_servlet->addServlet(uri, cb);
    }

    RPCDispatcher::ptr RPCDispatcher::GetCurrentRPCDispatcher() {
        return t_current_rpc_dispatcher;
    }

    RPCDispatcher::~RPCDispatcher() {

    }

    void RPCDispatcher::handle(Protocol::ptr request, Protocol::ptr response, Session::ptr session) {
        m_dispatch_servlet->handle(request, response, session);
    }

}

