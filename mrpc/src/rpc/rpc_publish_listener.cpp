//
// Created by baitianyu on 2/12/25.
//
#include "rpc/rpc_publish_listener.h"

namespace mrpc {

    PublishListener::PublishListener(NetAddr::ptr local_addr,
                                     PublishListener::callback call_back,
                                     EventLoop::ptr specific_eventloop) :
            TCPServer(local_addr, specific_eventloop), m_local_addr(local_addr), m_callback(call_back) {
        addServlet(RPC_REGISTER_PUBLISH_PATH, m_callback); // 加入publish处理的servlet
        start(); // 启动监听
    }

    PublishListener::~PublishListener() {
        DEBUGLOG("~PublishListener");
    }


}