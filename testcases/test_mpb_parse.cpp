//
// Created by baitianyu on 2/18/25.
//
#include <iostream>
#include "net/protocol/mpb/mpb_define.h"
#include "net/protocol/mpb/mpb_parse.h"

using namespace mrpc;

int main() {
    MPbManager::body_type body;
    body["service_name"] = "Order";
    auto request = std::make_shared<MPbProtocol>();
    MPbManager::createRequest(request, MSGType::RPC_CLIENT_REGISTER_DISCOVERY_REQUEST, body);
    std::cout << request->m_magic << " " << request->m_type << " "
              << request->m_msg_id << " " << request->m_body << std::endl;

    auto strs = request->toString();

    auto parser = std::make_shared<MPbProtocolParser>();
    parser->parse(strs);

    std::cout << parser->getRequest()->m_magic << " " << parser->getRequest()->m_type << " "
              << parser->getRequest()->m_msg_id << " " << parser->getRequest()->m_body << std::endl;
}