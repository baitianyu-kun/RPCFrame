//
// Created by baitianyu on 7/19/24.
//
#include "net/io_thread_pool.h"

namespace rocket {


    IOThreadPool::IOThreadPool(int size) {
        m_io_thread_pools.resize(size);
        for (auto &m_io_thread: m_io_thread_pools) {
            m_io_thread = std::move(std::unique_ptr<IOThread>(new IOThread()));
        }
    }

    IOThreadPool::~IOThreadPool() {

    }

    void IOThreadPool::start() {
        for (const auto &m_io_thread: m_io_thread_pools) {
            m_io_thread->start();
        }
    }

    void IOThreadPool::join() {
        for (const auto &m_io_thread: m_io_thread_pools) {
            m_io_thread->join();
        }
    }

    std::unique_ptr<IOThread> &IOThreadPool::getIOThread() {
        if (m_index == m_io_thread_pools.size()) {
            m_index = 0;
        }
        return m_io_thread_pools[m_index++];
    }

}
