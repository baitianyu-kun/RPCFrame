//
// Created by baitianyu on 2/8/25.
//
#include <unordered_map>
#include "net/protocol/http/http_servlet.h"

#define RPC_METHOD_PATH "/method"
#define RPC_REGISTER_UPDATE_SERVER_PATH "/update"
#define RPC_SERVER_REGISTER_PATH "/register"
#define RPC_CLIENT_REGISTER_DISCOVERY_PATH "/discovery"

namespace rocket {

    // 把负载均衡挪到客户端上
    class RPCDispatcher {
    public:
        enum class MSGType : uint8_t {
            // 调用RPC方法后的请求与响应
            RPC_METHOD_REQUEST,
            RPC_METHOD_RESPONSE,

            // 心跳机制，定时更新，只不过没有发送心跳包
            RPC_REGISTER_UPDATE_SERVER_REQUEST, // 注册中心请求服务端更新信息
            RPC_REGISTER_UPDATE_SERVER_RESPONSE, // 服务端相应注册中心更新信息

            // 服务注册
            RPC_SERVER_REGISTER_REQUEST, // 注册到注册中心
            RPC_SERVER_REGISTER_RESPONSE, // 注册完成后给予回应

            // 服务发现
            RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, // 客户端从注册中心中进行请求
            RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE, // 注册中心给客户端响应
        };

        using ptr = std::shared_ptr<RPCDispatcher>;

        RPCDispatcher();

        ~RPCDispatcher();

        void handle(HTTPRequest::ptr request, HTTPResponse::ptr response);

        static thread_local ptr t_current_rpc_dispatcher;

        static ptr GetCurrentRPCDispatcher();

    private:
        // 适用于小型、生命周期明确的变量。在栈上创建
        // 适用于较大对象、需要动态管理生命周期的对象，在堆上创建
        DispatchServlet::ptr m_dispatch_servlet;

    private:
        void registerUpdateServer(HTTPRequest::ptr request, HTTPResponse::ptr response);

        void serverRegister(HTTPRequest::ptr request, HTTPResponse::ptr response);

        void clientRegister(HTTPRequest::ptr request, HTTPResponse::ptr response);

        void clientServer(HTTPRequest::ptr request, HTTPResponse::ptr response);

    public:
        using body_type = std::unordered_map<std::string, std::string>;

        static HTTPRequest::ptr createRequest(MSGType type, body_type body);

        static HTTPResponse::ptr createResponse(MSGType type, body_type body);

    private:

        static HTTPRequest::ptr createMethodRequest(body_type body);

        static HTTPRequest::ptr createUpdateRequest(body_type body);

        static HTTPRequest::ptr createRegisterRequest(body_type body);

        static HTTPRequest::ptr createDiscoveryRequest(body_type body);

        static HTTPResponse::ptr createMethodResponse(body_type body);

        static HTTPResponse::ptr createUpdateResponse(body_type body);

        static HTTPResponse::ptr createRegisterResponse(body_type body);

        static HTTPResponse::ptr createDiscoveryResponse(body_type body);
    };
}