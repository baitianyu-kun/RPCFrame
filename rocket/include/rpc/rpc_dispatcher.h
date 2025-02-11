//
// Created by baitianyu on 2/8/25.
//
#include <unordered_map>
#include "net/protocol/http/http_servlet.h"

namespace rocket {

    class RPCDispatcher {
    public:
        using ptr = std::shared_ptr<RPCDispatcher>;

        RPCDispatcher();

        ~RPCDispatcher();

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response, HTTPSession::ptr session);

        static ptr t_current_rpc_dispatcher;

        static ptr GetCurrentRPCDispatcher();

        void addServlet(const std::string &uri, Servlet::ptr slt);

        void addServlet(const std::string &uri, CallBacksServlet::callback cb);

    private:
        // 适用于小型、生命周期明确的变量。在栈上创建
        // 适用于较大对象、需要动态管理生命周期的对象，在堆上创建
        DispatchServlet::ptr m_dispatch_servlet;
    };
}