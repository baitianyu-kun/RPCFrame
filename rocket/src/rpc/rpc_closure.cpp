//
// Created by baitianyu on 2/10/25.
//
#include "rpc/rpc_closure.h"
#include "common/log.h"

namespace rocket {
    rocket::RPCClosure::~RPCClosure() {
        DEBUGLOG("~RPCClosure");
    }
}