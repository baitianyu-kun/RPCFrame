//
// Created by baitianyu on 9/2/24.
//
#include "coroutine/coroutine_pool.h"

namespace rocket {

    CoroutinePool::coroutine_pool_sptr_t_ CoroutinePool::t_coroutine_pool = nullptr;

    CoroutinePool::coroutine_pool_sptr_t_ CoroutinePool::GetCoroutinePool() {
        // 写成懒汉式，如果写成饿汉的话，可能在上面make的时候logger还没初始化，造成memory里面使用到的logger发生段错误。饿汉算是延迟启动
        if (!t_coroutine_pool) {
            t_coroutine_pool = std::make_shared<CoroutinePool>(COR_POOL_SIZE, COR_POOL_STACK_SIZE);
        }
        return t_coroutine_pool;
    }

    CoroutinePool::CoroutinePool(int pool_size, int stack_size /*1024 * 128*/) : m_pool_size(pool_size),
                                                                                 m_stack_size(stack_size) {
        // 先设置main coroutine
        Coroutine::GetCurrentCoroutine();
        m_memory_pool.emplace_back(std::make_shared<Memory>(stack_size, pool_size)); // 协程池里面有多少个，就是多少个block
        auto tmp_memory = m_memory_pool[0];
        for (int i = 0; i < pool_size; ++i) {
            auto cor = std::make_shared<Coroutine>(stack_size, tmp_memory->getBlock());
            cor->setIndex(i);
            m_free_cors.emplace_back(std::make_pair(cor, false)); // 标记是否被使用该cor
        }
    }

    CoroutinePool::~CoroutinePool() {

    }

    Coroutine::coroutine_sptr_t_ CoroutinePool::getCoroutineInstance() {
        // 先去reuse已经分配了内存但是为false的协程，因为重新malloc消耗比较大，就应该直接使用之前剩下的不用的
        ScopeMutext<Mutex> lock(m_mutex);
        for (int i = 0; i < m_pool_size; ++i) {
            if (!m_free_cors[i].first->getIsInCoFunc() && !m_free_cors[i].second) {
                // 不在co func中，并且没有在使用，证明是已分配但是没有使用的协程
                m_free_cors[i].second = true;
                auto cor = m_free_cors[i].first;
                lock.unlock();
                return cor;
            }
        }
        // 如果不存在之前剩下的不使用的，那么就只能重新进行分配了
        // 看memory pool里面有没有不用的block
        for (int i = 0; i < m_memory_pool.size(); ++i) {
            auto free_block = m_memory_pool[i]->getBlock();
            if (free_block) {
                auto cor = std::make_shared<Coroutine>(m_stack_size, free_block);
                return cor;
            }
        }

        // 也没有不用的block，只能在memory pool里面创建新的memory
        m_memory_pool.emplace_back(std::make_shared<Memory>(m_stack_size, m_pool_size));
        return std::make_shared<Coroutine>(m_stack_size, m_memory_pool[m_memory_pool.size() - 1]->getBlock());
    }

    void CoroutinePool::returnCoroutine(Coroutine::coroutine_sptr_t_ cor) {
        auto index = cor->getIndex();
        if (index >= 0 && index < m_pool_size) {
            m_free_cors[index].second = false;
        } else {
            // auto tmp_memory = m_memory_pool[0]; 说明不在m memory pool[0]中，是getCoroutineInstance中最后新建的
            // return std::make_shared<Coroutine>(m_stack_size, m_memory_pool[m_memory_pool.size() - 1]->getBlock());
            // 当时没有设置index，默认为-1
            for (int i = 1; i < m_memory_pool.size(); ++i) {
                if (m_memory_pool[i]->hasBlock(cor->getStackPtr())) {
                    m_memory_pool[i]->backBlock(cor->getStackPtr());
                }
            }
        }
    }


}