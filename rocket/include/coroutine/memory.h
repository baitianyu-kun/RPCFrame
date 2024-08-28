//
// Created by baitianyu on 8/27/24.
//

#ifndef RPCFRAME_MEMORY_H
#define RPCFRAME_MEMORY_H

#include <memory>
#include <atomic>
#include <vector>
#include "common/mutex.h"

namespace rocket {

    class Memory {
    public:
        using memory_sptr_t_ = std::shared_ptr<Memory>;
    public:
        // 根据block size和count来决定分配多少内存
        Memory(int block_size, int block_count);

        ~Memory();

        int getRefCount();

        char *getStart();

        char *getEnd();

        char *getBlock();

        void backBlock(char *s); // 已分配的返回给内存

        bool hasBlock(char *s);

    private:
        int m_block_size{0};
        int m_block_count{0};

        int m_total_size{0};
        char *m_start{nullptr};
        char *m_end{nullptr};

        std::atomic<int> m_ref_counts;
        std::vector<bool> m_blocks; // 大小为m_block_count，代表分配了还是没分配
        Mutex m_mutex;

    };

}

#endif //RPCFRAME_MEMORY_H
