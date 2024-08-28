//
// Created by baitianyu on 8/27/24.
//
#include <cassert>
#include "coroutine/memory.h"
#include "common/log.h"

namespace rocket {

    rocket::Memory::Memory(int block_size, int block_count) : m_block_size(block_size), m_block_count(block_count) {
        m_total_size = block_size * block_count;
        // 分配内存，malloc只分配，不调用构造函数等东西，并且分配完是void*，需要转换为要用的
        m_start = (char *) malloc(m_total_size);
        assert(m_start != (void *) -1); // 不为-1证明分配成功
        INFOLOG("success mmap %d bytes memory", m_total_size);
        m_end = m_start + m_total_size - 1;
        m_blocks.resize(m_block_count);
        for (auto i = 0; i < m_blocks.size(); ++i) {
            // vector bool返回的是proxy对象，对这个对象修改也可以修改vector里面的值，就是注意bool *pb = &v[0];这样是不对的
            // 不能取地址，原因是&v[0]的类型是reference*类型不是bool*类型。vector bool里面存的是二进制位，而不是bool
            m_blocks[i] = false;
        }
        m_ref_counts = 0; // 有多少使用的
    }

    Memory::~Memory() {
        // 释放空间
        if (!m_start || m_start == (void *) -1) {
            return; // m_start为空，或者未成功分配内存
        }
        free(m_start);
        INFOLOG("~success free mmap %d bytes memeory", m_total_size);
        m_start = nullptr;
        m_ref_counts = 0;
    }

    int Memory::getRefCount() {
        return m_ref_counts;
    }

    char *Memory::getStart() {
        return m_start;
    }

    char *Memory::getEnd() {
        return m_end;
    }

    char *Memory::getBlock() {
        ScopeMutext<Mutex> lock(m_mutex);
        int free_block_index = -1;
        // 遍历blocks看哪个没有被分配
        for (int i = 0; i < m_blocks.size(); ++i) {
            if (m_blocks[i] == false) {
                m_blocks[i] = true;
                free_block_index = i;
                break;
            }
        }
        lock.unlock();
        if (free_block_index == -1) {
            return nullptr;
        }
        m_ref_counts++; // 使用的block数+1
        return m_start + (free_block_index * m_block_size);
    }

    void Memory::backBlock(char *s) {
        if (s > m_end || s < m_start) {
            ERRORLOG("this block is not belong to this Memory");
            return;
        }
        auto back_block_index = (s - m_start) / m_block_size;
        ScopeMutext<Mutex> lock(m_mutex);
        m_blocks[back_block_index] = false;
        m_mutex.unlock();
        m_ref_counts--;
    }

    bool Memory::hasBlock(char *s) {
        return ((s >= m_start) && (s <= m_end));
    }

}



