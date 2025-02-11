//
// Created by baitianyu on 25-2-10.
//
#include "rpc/register_center.h"
#include "common/mutex.h"

namespace rocket {

    RegisterCenter::RegisterCenter(NetAddr::ptr local_addr) : TCPServer(local_addr), m_local_addr(local_addr) {
        initServlet();
    }

    RegisterCenter::~RegisterCenter() {
        DEBUGLOG("~RegisterCenter");
    }

    void RegisterCenter::initServlet() {
        // 客户端访问注册中心
        addServlet(RPC_CLIENT_REGISTER_DISCOVERY_PATH, std::bind(&RegisterCenter::handleClientDiscovery, this,
                                                                 std::placeholders::_1,
                                                                 std::placeholders::_2,
                                                                 std::placeholders::_3));
        // 服务端访问注册中心
        addServlet(RPC_SERVER_REGISTER_PATH, std::bind(&RegisterCenter::handleServerRegister, this,
                                                       std::placeholders::_1,
                                                       std::placeholders::_2,
                                                       std::placeholders::_3));
    }

    void RegisterCenter::startRegisterCenter() {
        start();
    }

    void RegisterCenter::handleServerRegister(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                              HTTPSession::ptr session) {
        RWMutex::WriteLock lock(m_mutex);
    }

    void RegisterCenter::handleClientDiscovery(HTTPRequest::ptr request, HTTPResponse::ptr response,
                                               HTTPSession::ptr session) {
        RWMutex::ReadLock lock(m_mutex);
    }

    void RegisterCenter::notifyClientServerFailed() {

    }

    void RegisterCenter::publishClientMessage() {

    }
}

