//
// Created by baitianyu on 7/20/24.
//
#include "net/fd_event_pool.h"

namespace rocket {
    // 单例模式，懒汉类型，用的收才进行创建，线程不安全，假设两个线程，一个判断为nullptr
    // 另一个判断不为nullptr，那么就可能出现创建了两个指针，新的把旧的给覆盖了
    // static std::unique_ptr<FDEventPool> g_fd_event_pool = nullptr;

    // 单例模式，饿汉类型，线程安全
    // 提高封装性更改为在类里面，而不是在整个文件的全局变量的单例模式，虽然后者也可以
    std::unique_ptr<FDEventPool> FDEventPool::g_fd_event_pool =
            std::move(std::unique_ptr<FDEventPool>(new FDEventPool(MAX_FD_EVENT_POOL_SIZE)));

    std::unique_ptr<FDEventPool> &FDEventPool::GetFDEventPool() {
        //这里感觉会出现线程安全问题，可以用call once方法，或者加锁来实现
        // 或者使用饿汉式
        // if (g_fd_event_pool != nullptr) {
        //     return g_fd_event_pool;
        // }
        // g_fd_event_pool = std::move(std::unique_ptr<FDEventPool>(new FDEventPool(MAX_FD_EVENT_POOL_SIZE)));
        // return g_fd_event_pool;

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
    FDEvent::fd_event_sptr_t_ FDEventPool::getFDEvent(int fd) {
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
}


