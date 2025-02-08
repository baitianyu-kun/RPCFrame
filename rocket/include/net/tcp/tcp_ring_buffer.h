//
// Created by baitianyu on 2/6/25.
//

#ifndef TCP_RING_BUFFER_H
#define TCP_RING_BUFFER_H

#include <cassert>
#include <cstring>
#include <memory>
#include <vector>
#include "net/tcp/abstract_tcp_buffer.h"

/**
 * Ref: https://github.com/AndersKaloer/Ring-Buffer
 * Used as a modulo operator
 * as <tt> a % b = (a & (b − 1)) </tt>
 * where \c a is a positive index in the buffer and
 * \c b is the (power of two) size of the buffer.
 */

#define RING_BUFFER_IS_POWER_OF_TWO(buffer_size) ((buffer_size & (buffer_size - 1)) == 0)
#define RING_BUFFER_ASSERT(x) assert(x)
#define MAX_TCP_BUFFER_SIZE 2048

namespace rocket {
    class TCPRingBuffer : TCPBuffer {
    public:
        using ptr = std::shared_ptr<TCPRingBuffer>;

        explicit TCPRingBuffer(int buff_size);

        ~TCPRingBuffer();

        void WriteRingBuffQueue(char data);

        void WriteRingBuffQueueArr(char *data, int size);

        int ReadRingBuffQueue(char *data);

        int ReadRingBuffQueueArr(char *data, int size);

        std::vector<char> ReadRingBuffQueueArrVec(int size);

        int RingBuffIsFull() const { return ((m_write_idx - m_read_idx) & (m_buffer_mask)) == m_buffer_mask; }

        int RingBuffIsEmpty() const { return m_write_idx == m_read_idx; }

        int RingBuffNumItems() const { return (m_write_idx - m_read_idx) & (m_buffer_mask); }

    public:
        void readFromBuffer(std::vector<char> &read_buff, int size) override;

        void writeToBuffer(char *buff, int size) override;

        void writeToBuffer(const char *buff, int size) override;

        int getReadIndex() const override { return m_read_idx; }

        int getWriteIndex() const override { return m_write_idx; }

        int readAbleSize() const override { return RingBuffNumItems(); }

        // 环形会自动覆盖，所以最多可以写入这么多个，用来给read需要指定读取大小来使用
        int writeAbleSize() const override { return m_max_size - 1; };

    private:
        int m_read_idx{0};
        int m_write_idx{0};
        int m_max_size{0};
        std::unique_ptr<char[]> m_buffer;
        int m_buffer_mask{0};
    };
} // namespace rocket

#endif  // TCP_RING_BUFFER_H
