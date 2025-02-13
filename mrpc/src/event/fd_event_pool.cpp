//
// Created by baitianyu on 7/20/24.
//
#include "event/fd_event_pool.h"
#include "common/log.h"

namespace mrpc {

    std::unique_ptr<FDEventPool> FDEventPool::g_fd_event_pool = std::make_unique<FDEventPool>(MAX_FD_EVENT_POOL_SIZE);

    std::unique_ptr<FDEventPool> &FDEventPool::GetFDEventPool() {
        return g_fd_event_pool;
    }

    FDEventPool::FDEventPool(int size) : m_size(size) {
        for (size_t i = 0; i < size; ++i) {
            // 以fd的值作为下标
            m_fd_pool.emplace_back(std::make_shared<FDEvent>(i));
        }
    }

    FDEventPool::~FDEventPool() {

    }

    // 有可能多个进程会访问这个函数，所以要加锁
    FDEvent::ptr FDEventPool::getFDEvent(int fd) {
        ScopeMutext<Mutex> lock(m_mutex);
        if ((size_t) fd < m_fd_pool.size()) {
            return m_fd_pool[fd];
        }
        // 否则需要扩展这个event pool，扩大为原来的1.5倍
        auto new_size = int(fd * 1.5);
        for (size_t i = m_fd_pool.size(); i < new_size; i++) {
            m_fd_pool.emplace_back(std::make_shared<FDEvent>(i));
        }
        return m_fd_pool[fd];
    }

    void FDEventPool::deleteFDEvent(int fd) {
        if (m_fd_pool[fd]) {
            m_fd_pool[fd].reset();
            m_fd_pool.erase(m_fd_pool.begin() + fd);
        }
    }
}


