//
// Created by baitianyu on 25-2-10.
//
#include "rpc/register_center.h"

namespace rocket {

    RegisterCenter::RegisterCenter(NetAddr::ptr local_addr) : TCPServer(local_addr), m_local_addr(local_addr) {
        initServlet();
    }

    RegisterCenter::~RegisterCenter() {
        DEBUGLOG("~RegisterCenter");
    }

    void RegisterCenter::initServlet() {
        // 客户端访问注册中心
        // 服务端访问注册中心

    }

    void RegisterCenter::startRegisterCenter() {

    }

    void RegisterCenter::handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                              HTTPSession::ptr session) {

    }

    void RegisterCenter::handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                               HTTPSession::ptr session) {

    }

    void RegisterCenter::notifyClientServerFailed() {

    }

    void RegisterCenter::publishClientMessage() {

    }
}

