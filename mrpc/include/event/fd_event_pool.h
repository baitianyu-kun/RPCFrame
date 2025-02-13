//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_FD_EVENT_POOL_H
#define RPCFRAME_FD_EVENT_POOL_H

#include <vector>
#include "common/mutex.h"
#include "event/fd_event.h"

#define MAX_FD_EVENT_POOL_SIZE 128

namespace mrpc {
    class FDEventPool {
    public:

        static std::unique_ptr<FDEventPool> g_fd_event_pool;

        static std::unique_ptr<FDEventPool> &GetFDEventPool();

    public:
        explicit FDEventPool(int size);

        ~FDEventPool();

        FDEvent::ptr getFDEvent(int fd);

        void deleteFDEvent(int fd);

    private:
        int m_size{0};
        std::vector<FDEvent::ptr> m_fd_pool;
        Mutex m_mutex;
    };
}

#endif //RPCFRAME_FD_EVENT_POOL_H
