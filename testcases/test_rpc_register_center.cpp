//
// Created by baitianyu on 25-2-12.
//
#include "rpc/register_center.h"

using namespace rocket;

int main() {
    Config::SetGlobalConfig("../conf/rocket.xml");
    Logger::InitGlobalLogger(0);
    auto local_addr = std::make_shared<IPNetAddr>("127.0.0.1", 22225);

    auto register_center = std::make_shared<RegisterCenter>(local_addr);

    register_center->startRegisterCenter();
    return 0;
}