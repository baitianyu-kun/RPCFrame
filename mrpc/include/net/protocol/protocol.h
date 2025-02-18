//
// Created by baitianyu on 2/7/25.
//

#ifndef RPCFRAME_PROTOCOL_H
#define RPCFRAME_PROTOCOL_H

#include <memory>
#include <unordered_map>
#include "common/config.h"
#include "net/tcp/net_addr.h"

#define RPC_METHOD_PATH Config::GetGlobalConfig()->m_rpc_method_path
#define RPC_REGISTER_HEART_SERVER_PATH Config::GetGlobalConfig()->m_rpc_register_heart_server_path
#define RPC_SERVER_REGISTER_PATH Config::GetGlobalConfig()->m_rpc_server_register_path
#define RPC_CLIENT_REGISTER_DISCOVERY_PATH Config::GetGlobalConfig()->m_rpc_client_register_discovery_path
#define RPC_REGISTER_SUBSCRIBE_PATH Config::GetGlobalConfig()->m_register_subscribe_path
#define RPC_REGISTER_PUBLISH_PATH Config::GetGlobalConfig()->m_register_publish_path

namespace mrpc {

    static std::string g_CRLF = "\r\n";
    static std::string g_CRLF_DOUBLE = "\r\n\r\n";

    static std::string content_type_text = "text/html;charset=utf-8";
    static const char *default_html_template = "<html><body><h1>hello</h1><p>rpc</p></body></html>";

    using body_type = std::unordered_map<std::string, std::string>;

    enum class MSGType : uint8_t {
        // 调用RPC方法后的请求与响应
        RPC_METHOD_REQUEST,
        RPC_METHOD_RESPONSE,

        // 心跳机制
        RPC_REGISTER_HEART_SERVER_REQUEST, // 注册中心向服务端发送heart pack
        RPC_REGISTER_HEART_SERVER_RESPONSE, // 服务端向注册中心回应heart pack

        // 服务注册
        RPC_SERVER_REGISTER_REQUEST, // 注册到注册中心
        RPC_SERVER_REGISTER_RESPONSE, // 注册完成后给予回应

        // 服务发现
        RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, // 客户端从注册中心中进行请求
        RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE, // 注册中心给客户端响应

        // 服务订阅
        RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST, // 客户端订阅相关service name
        RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE, // 该service name中服务器列表发生变化主动通知客户端

        // 服务推送
        RPC_REGISTER_CLIENT_PUBLISH_REQUEST, // 注册中心推送消给息客户端
        RPC_REGISTER_CLIENT_PUBLISH_RESPONSE, // 客户端收到推送
    };

    static MSGType requestPathToMSGType(const std::string &path) {
        if (path == RPC_METHOD_PATH) {
            return MSGType::RPC_METHOD_REQUEST;
        } else if (path == RPC_REGISTER_HEART_SERVER_PATH) {
            return MSGType::RPC_REGISTER_HEART_SERVER_REQUEST;
        } else if (path == RPC_SERVER_REGISTER_PATH) {
            return MSGType::RPC_SERVER_REGISTER_REQUEST;
        } else if (path == RPC_CLIENT_REGISTER_DISCOVERY_PATH) {
            return MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST;
        } else if (path == RPC_REGISTER_SUBSCRIBE_PATH) {
            return MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST;
        } else if (path == RPC_REGISTER_PUBLISH_PATH) {
            return MSGType::RPC_REGISTER_CLIENT_PUBLISH_REQUEST;
        }
    }

    static std::string msgTypeToPath(MSGType type){
        switch (type) {
            case MSGType::RPC_METHOD_REQUEST:
                return RPC_METHOD_PATH;
            case MSGType::RPC_REGISTER_HEART_SERVER_REQUEST:
                return RPC_REGISTER_HEART_SERVER_PATH;
            case MSGType::RPC_SERVER_REGISTER_REQUEST:
                return RPC_SERVER_REGISTER_PATH;
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST:
                return RPC_CLIENT_REGISTER_DISCOVERY_PATH;
            case MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST:
                return RPC_REGISTER_SUBSCRIBE_PATH;
            case MSGType::RPC_REGISTER_CLIENT_PUBLISH_REQUEST:
                return RPC_REGISTER_PUBLISH_PATH;
            case MSGType::RPC_METHOD_RESPONSE:
                return RPC_METHOD_PATH;
            case MSGType::RPC_REGISTER_HEART_SERVER_RESPONSE:
                return RPC_REGISTER_HEART_SERVER_PATH;
            case MSGType::RPC_SERVER_REGISTER_RESPONSE:
                return RPC_SERVER_REGISTER_PATH;
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE:
                return RPC_CLIENT_REGISTER_DISCOVERY_PATH;
            case MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE:
                return RPC_REGISTER_SUBSCRIBE_PATH;
            case MSGType::RPC_REGISTER_CLIENT_PUBLISH_RESPONSE:
                return RPC_REGISTER_PUBLISH_PATH;
        }
    }

    enum ProtocolType {
        MPb_Protocol = 1,
        HTTP_Protocol = 2
    };

    class Protocol {
    public:
        using ptr = std::shared_ptr<Protocol>;

        Protocol() = default;

        virtual ~Protocol() = default;

        virtual std::string toString() = 0;

    public:
        std::string m_msg_id; // 请求号，唯一标识一个请求或者响应
        MSGType m_type;
        std::unordered_map<std::string, std::string> m_body_data_map; // split后的body map
    };

    class Session {
    public:
        using ptr = std::shared_ptr<Session>;

        Session(NetAddr::ptr local_addr,
                NetAddr::ptr peer_addr) : m_local_addr(local_addr), m_peer_addr(peer_addr) {}

        ~Session() = default;

        NetAddr::ptr getLocalAddr() const { return m_local_addr; }

        NetAddr::ptr getPeerAddr() const { return m_peer_addr; }

    private:
        NetAddr::ptr m_local_addr;
        NetAddr::ptr m_peer_addr;
    };
}

#endif //RPCFRAME_PROTOCOL_H
