//
// Created by baitianyu on 2/18/25.
//

#ifndef RPCFRAME_MPB_DEFINE_H
#define RPCFRAME_MPB_DEFINE_H

#include <memory>
#include <cstdint>
#include <unordered_map>

/*
 * +-----------------------------------------------------------------------------+
 * |  magic |  type  |  msg id len  |  msg id  |  content len  |    content      |
 * +-----------------------------------------------------------------------------+
 * 魔法数，1字节
 * 请求类型，1字节
 * msg id len，32位，4字节
 * msg id，char[]类型，不固定
 * content len，32位，4字节
 * content，n个字节
 */
namespace mrpc {

    extern std::string g_CRLF2;
    extern std::string g_CRLF_DOUBLE2;
    extern std::string content_type_text2;
    extern const char *default_html_template2;
    static uint8_t MAGIC = 0xbc;

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

    class MPbProtocol {
    public:
        using ptr = std::shared_ptr<MPbProtocol>;

    public:
        std::string toString();

    public:
        uint8_t m_magic = MAGIC;
        uint8_t m_type;
        std::string m_msg_id;
        std::string m_body;
        std::unordered_map<std::string, std::string> m_body_data_map;
    };

    class MPbManager {
    public:
        using body_type = std::unordered_map<std::string, std::string>;

        static void createRequest(MPbProtocol::ptr request, MSGType type, body_type &body);

        static void createResponse(MPbProtocol::ptr response, MSGType type, body_type &body);

    private:

        static void createMethodRequest(MPbProtocol::ptr request, body_type &body);

        static void createHeartRequest(MPbProtocol::ptr request, body_type &body);

        static void createRegisterRequest(MPbProtocol::ptr request, body_type &body);

        static void createDiscoveryRequest(MPbProtocol::ptr request, body_type &body);

        static void createSubscribeRequest(MPbProtocol::ptr request, body_type &body);

        static void createPublishRequest(MPbProtocol::ptr request, body_type &body);

        static void createMethodResponse(MPbProtocol::ptr response, body_type &body);

        static void createHeartResponse(MPbProtocol::ptr response, body_type &body);

        static void createRegisterResponse(MPbProtocol::ptr response, body_type &body);

        static void createDiscoveryResponse(MPbProtocol::ptr response, body_type &body);

        static void createSubscribeResponse(MPbProtocol::ptr response, body_type &body);

        static void createPublishResponse(MPbProtocol::ptr response, body_type &body);

    };
}

#endif //RPCFRAME_MPB_DEFINE_H
