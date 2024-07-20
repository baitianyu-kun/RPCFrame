//
// Created by baitianyu on 7/20/24.
//
#include "net/fd_event_pool.h"

namespace rocket{

    std::unique_ptr<FDEventPool> FDEventPool::GetFDEventPool() {
        return std::unique_ptr<FDEventPool>();
    }

    FDEventPool::FDEventPool(int size) {

    }

    FDEventPool::~FDEventPool() {

    }

    FDEvent::fd_event_sptr_t_ FDEventPool::getFDEvent(int fd) {
        return {};
    }
}


