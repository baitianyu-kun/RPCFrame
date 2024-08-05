//
// Created by baitianyu on 7/25/24.
//
#include "net/rpc/rpc_closure.h"
#include "common/log.h"

namespace rocket{
    rocket::RPCClosure::~RPCClosure() {
        DEBUGLOG("~RPCClosure");
    }
}


