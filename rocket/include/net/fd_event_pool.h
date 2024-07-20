//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_FD_EVENT_POOL_H
#define RPCFRAME_FD_EVENT_POOL_H

#include <vector>
#include "common/mutex.h"
#include "net/fd_event.h"

namespace rocket {
    class FDEventPool {
    public:
        static std::unique_ptr<FDEventPool> GetFDEventPool();

    public:
        explicit FDEventPool(int size);

        ~FDEventPool();

        FDEvent::fd_event_sptr_t_ getFDEvent(int fd);

    private:
        int m_size{0};
        std::vector<FDEvent::fd_event_sptr_t_> m_fd_pool;
        Mutex m_mutex;
    };
}

#endif //RPCFRAME_FD_EVENT_POOL_H
