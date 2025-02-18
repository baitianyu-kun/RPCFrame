//
// Created by baitianyu on 2/18/25.
//
#include <arpa/inet.h>
#include "net/protocol/mpb/mpb_define.h"
#include "common/string_util.h"

namespace mrpc {

    std::string MPbProtocol::toString() {
        size_t all_len =
                sizeof(m_magic) + sizeof(m_type) + 2 * sizeof(uint32_t) + m_msg_id.length() + m_body.length();
        std::vector<char> buf(all_len);
        char *tmp = buf.data();
        // magic
        memcpy(tmp, &m_magic, sizeof(m_magic));
        tmp += sizeof(m_magic);
        // type
        memcpy(tmp, &m_type, sizeof(m_type));
        tmp += sizeof(m_type);
        // msg id len
        uint32_t msg_id_len = static_cast<uint32_t>(m_msg_id.length());
        uint32_t msg_id_len_net = htonl(msg_id_len);
        memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
        tmp += sizeof(msg_id_len_net);
        // msg id
        if (!m_msg_id.empty()) {
            memcpy(tmp, m_msg_id.data(), m_msg_id.length());
            tmp += m_msg_id.length();
        }
        // content_len
        uint32_t content_len = static_cast<uint32_t>(m_body.length());
        uint32_t content_len_net = htonl(content_len);
        memcpy(tmp, &content_len_net, sizeof(content_len_net));
        tmp += sizeof(content_len_net);
        // content
        if (!m_body.empty()) {
            memcpy(tmp, m_body.data(), m_body.length());
            tmp += m_body.length();
        }
        return {buf.data(), all_len};
    }

    void MPbManager::createRequest(MPbProtocol::ptr request, MSGType type, body_type &body) {
        switch (type) {
            case MSGType::RPC_METHOD_REQUEST:
                createMethodRequest(request, body);
                return;
            case MSGType::RPC_REGISTER_HEART_SERVER_REQUEST:
                createHeartRequest(request, body);
                return;
            case MSGType::RPC_SERVER_REGISTER_REQUEST:
                createRegisterRequest(request, body);
                return;
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST:
                createDiscoveryRequest(request, body);
                return;
            case MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST:
                createSubscribeRequest(request, body);
                return;
            case MSGType::RPC_REGISTER_CLIENT_PUBLISH_REQUEST:
                createPublishRequest(request, body);
                return;
        }
    }

    void MPbManager::createResponse(MPbProtocol::ptr response, MSGType type, body_type &body) {
        switch (type) {
            case MSGType::RPC_METHOD_RESPONSE:
                createMethodResponse(response, body);
                return;
            case MSGType::RPC_REGISTER_HEART_SERVER_RESPONSE:
                createHeartResponse(response, body);
                return;
            case MSGType::RPC_SERVER_REGISTER_RESPONSE:
                createRegisterResponse(response, body);
                return;
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE:
                createDiscoveryResponse(response, body);
                return;
            case MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE:
                createSubscribeResponse(response, body);
                return;
            case MSGType::RPC_REGISTER_CLIENT_PUBLISH_RESPONSE:
                createPublishResponse(response, body);
                return;
        }
    }

    void MPbManager::createMethodRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_METHOD_REQUEST;
    }

    void MPbManager::createHeartRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_REGISTER_HEART_SERVER_REQUEST;
    }

    void MPbManager::createRegisterRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"] + g_CRLF
                               + "all_services_names:" + body["all_services_names"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_SERVER_REGISTER_REQUEST;
    }

    void MPbManager::createDiscoveryRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "service_name:" + body["service_name"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST;
    }

    void MPbManager::createSubscribeRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "service_name:" + body["service_name"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_REQUEST;
    }

    void MPbManager::createPublishRequest(MPbProtocol::ptr request, body_type &body) {
        request->m_msg_id = MSGIDUtil::GenerateMSGID();
        std::string body_str = "service_name:" + body["service_name"];
        request->m_body = body_str;
        request->m_type = MSGType::RPC_REGISTER_CLIENT_PUBLISH_REQUEST;
    }

    void MPbManager::createMethodResponse(MPbProtocol::ptr response, body_type &body) {
        response->m_msg_id = body["msg_id"];
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"];
        response->m_body = body_str;
        response->m_type = MSGType::RPC_METHOD_RESPONSE;
    }

    void MPbManager::createHeartResponse(MPbProtocol::ptr response, body_type &body) {
        response->m_msg_id = body["msg_id"];
        response->m_type = MSGType::RPC_REGISTER_HEART_SERVER_RESPONSE;
    }

    void MPbManager::createRegisterResponse(MPbProtocol::ptr response, body_type &body) {
        std::string body_str = "add_service_count:" + body["add_service_count"];
        response->m_body = body_str;
        response->m_msg_id = body["msg_id"];
        response->m_type = MSGType::RPC_SERVER_REGISTER_RESPONSE;
    }

    void MPbManager::createDiscoveryResponse(MPbProtocol::ptr response, body_type &body) {
        std::string body_str = "server_list:" + body["server_list"];
        response->m_body = body_str;
        response->m_msg_id = body["msg_id"];
        response->m_type = MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE;
    }

    void MPbManager::createSubscribeResponse(MPbProtocol::ptr response, body_type &body) {
        std::string body_str = "service_name:" + body["service_name"] + g_CRLF
                               + "subscribe_success:" + body["subscribe_success"];
        response->m_body = body_str;
        response->m_msg_id = body["msg_id"];
        response->m_type = MSGType::RPC_CLIENT_REGISTER_SUBSCRIBE_RESPONSE;
    }

    void MPbManager::createPublishResponse(MPbProtocol::ptr response, body_type &body) {
        response->m_msg_id = body["msg_id"];
        response->m_type = MSGType::RPC_REGISTER_CLIENT_PUBLISH_RESPONSE;
    }
}