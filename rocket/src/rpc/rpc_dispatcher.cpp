//
// Created by baitianyu on 2/8/25.
//
#include "rpc/rpc_dispatcher.h"
#include "common/msg_id_util.h"
#include "common/log.h"

namespace rocket {

    RPCDispatcher::ptr RPCDispatcher::t_current_rpc_dispatcher = std::make_shared<RPCDispatcher>();

    RPCDispatcher::RPCDispatcher() {
        // 把相应的处理方法注册到dispatcher servlet中
        // 使用占位符来提前占住参数，等调用时候，外部参数可以通过占位符来进行转发
        m_dispatch_servlet = std::make_shared<DispatchServlet>();
        m_dispatch_servlet->addServlet(RPC_METHOD_PATH,
                                       std::bind(&RPCDispatcher::clientServer, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
        m_dispatch_servlet->addServlet(RPC_SERVER_REGISTER_PATH,
                                       std::bind(&RPCDispatcher::serverRegister, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
        m_dispatch_servlet->addServlet(RPC_CLIENT_REGISTER_DISCOVERY_PATH,
                                       std::bind(&RPCDispatcher::clientRegister, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
        m_dispatch_servlet->addServlet(RPC_REGISTER_UPDATE_SERVER_PATH,
                                       std::bind(&RPCDispatcher::registerUpdateServer, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2));
    }

    // 做成单例模式，整个只需要一个dispatcher，多线程共享，由于添加删除只有主线程进行，子线程getServlet的时候不参与添加删除，所以暂时不需要进行加锁
    RPCDispatcher::ptr RPCDispatcher::GetCurrentRPCDispatcher() {
        return t_current_rpc_dispatcher;
    }

    RPCDispatcher::~RPCDispatcher() {
        DEBUGLOG("~RPCDispatcher");
    }

    void RPCDispatcher::handle(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        m_dispatch_servlet->handle(request, response);
    }

    // 从request中读取数据，并写入到response中
    void RPCDispatcher::registerUpdateServer(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        DEBUGLOG("========== registerUpdateServer success ==========");
    }

    void RPCDispatcher::serverRegister(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        DEBUGLOG("========== serverRegister success ==========");
    }

    void RPCDispatcher::clientRegister(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        DEBUGLOG("========== clientRegister success ==========");
    }

    void RPCDispatcher::clientServer(HTTPRequest::ptr request, HTTPResponse::ptr response) {
        DEBUGLOG("========== clientServer success ==========");
    }

    HTTPRequest::ptr RPCDispatcher::createRequest(RPCDispatcher::MSGType type, RPCDispatcher::body_type body) {
        switch (type) {
            case MSGType::RPC_METHOD_REQUEST:
                return createMethodRequest(body);
            case MSGType::RPC_REGISTER_UPDATE_SERVER_REQUEST:
                return createUpdateRequest(body);
            case MSGType::RPC_SERVER_REGISTER_REQUEST:
                return createRegisterRequest(body);
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST:
                return createDiscoveryRequest(body);
        }
    }

    HTTPResponse::ptr RPCDispatcher::createResponse(RPCDispatcher::MSGType type, RPCDispatcher::body_type body) {
        switch (type) {
            case MSGType::RPC_METHOD_RESPONSE:
                return createMethodResponse(body);
            case MSGType::RPC_REGISTER_UPDATE_SERVER_RESPONSE:
                return createUpdateResponse(body);
            case MSGType::RPC_SERVER_REGISTER_RESPONSE:
                return createRegisterResponse(body);
            case MSGType::RPC_CLIENT_REGISTER_DISCOVERY_RESPONSE:
                return createDiscoveryResponse(body);
        }
    }

    HTTPRequest::ptr RPCDispatcher::createMethodRequest(RPCDispatcher::body_type body) {
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"] + g_CRLF
                               + "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_METHOD_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr RPCDispatcher::createUpdateRequest(RPCDispatcher::body_type body) {
        std::string body_str = "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_REGISTER_UPDATE_SERVER_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr RPCDispatcher::createRegisterRequest(RPCDispatcher::body_type body) {
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"] + g_CRLF
                               + "all_method_full_names" + body["all_method_full_names"] + g_CRLF
                               + "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_SERVER_REGISTER_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPRequest::ptr RPCDispatcher::createDiscoveryRequest(RPCDispatcher::body_type body) {
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "msg_id:" + MSGIDUtil::GenerateMSGID();
        auto request = std::make_shared<HTTPRequest>();
        request->m_request_body = body_str;
        request->m_request_method = HTTPMethod::POST;
        request->m_request_version = "HTTP/1.1";
        request->m_request_path = RPC_CLIENT_REGISTER_DISCOVERY_PATH;
        request->m_request_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        request->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        return request;
    }

    HTTPResponse::ptr RPCDispatcher::createMethodResponse(RPCDispatcher::body_type body) {
        std::string body_str = "method_full_name:" + body["method_full_name"] + g_CRLF
                               + "pb_data:" + body["pb_data"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr RPCDispatcher::createUpdateResponse(RPCDispatcher::body_type body) {
        std::string body_str = "add_method_count:" + body["add_method_count"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr RPCDispatcher::createRegisterResponse(RPCDispatcher::body_type body) {
        std::string body_str = "add_method_count:" + body["add_method_count"] + g_CRLF
                               + "msg_id:" + body["msg_id"];
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }

    HTTPResponse::ptr RPCDispatcher::createDiscoveryResponse(RPCDispatcher::body_type body) {
        std::string body_str = "server_ip:" + body["server_ip"] + g_CRLF
                               + "server_port:" + body["server_port"] + g_CRLF
                               + "msg_id:" + body["msg_id"];;
        auto response = std::make_shared<HTTPResponse>();
        response->m_response_body = body_str;
        response->m_response_version = "HTTP/1.1";
        response->m_response_code = HTTPCode::HTTP_OK;
        response->m_response_info = HTTPCodeToString(HTTPCode::HTTP_OK);
        response->m_response_properties.m_map_properties["Content-Length"] = std::to_string(body_str.length());
        response->m_response_properties.m_map_properties["Content-Type"] = content_type_text;
        return response;
    }
}

