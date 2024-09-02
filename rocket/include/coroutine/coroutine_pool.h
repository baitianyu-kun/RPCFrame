//
// Created by baitianyu on 9/2/24.
//

#ifndef RPCFRAME_COROUTINE_POOL_H
#define RPCFRAME_COROUTINE_POOL_H

#include "common/log.h"
#include "coroutine/memory.h"
#include "common/mutex.h"

#define COR_POOL_SIZE 10
#define COR_POOL_STACK_SIZE 1024*128

namespace rocket {

    class CoroutinePool {
    public:
        using coroutine_pool_sptr_t_ = std::shared_ptr<CoroutinePool>;

        static coroutine_pool_sptr_t_ t_coroutine_pool;

        static coroutine_pool_sptr_t_ GetCoroutinePool();

    public:
        explicit CoroutinePool(int pool_size, int stack_size = 1024 * 128);

        ~CoroutinePool();

        Coroutine::coroutine_sptr_t_ getCoroutineInstance();

        void returnCoroutine(Coroutine::coroutine_sptr_t_ cor);

    private:
        int m_pool_size{0};
        int m_stack_size{0};
        std::vector<std::pair<Coroutine::coroutine_sptr_t_, bool>> m_free_cors;
        Mutex m_mutex;
        std::vector<Memory::memory_sptr_t_> m_memory_pool;
    };


}

#endif //RPCFRAME_COROUTINE_POOL_H
