//
// Created by baitianyu on 2/18/25.
//

#ifndef RPCFRAME_MPB_DEFINE_H
#define RPCFRAME_MPB_DEFINE_H

#include <memory>
#include <cstdint>
#include <unordered_map>
#include "net/protocol/protocol.h"

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

    static uint8_t MAGIC = 0xbc;

    class MPbProtocol : public Protocol {
    public:
        using ptr = std::shared_ptr<MPbProtocol>;

    public:
        std::string toString() override;

    public:
        uint8_t m_magic = MAGIC;
        std::string m_body;
    };

    class MPbManager {
    public:

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
