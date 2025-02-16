//
// Created by baitianyu on 2/6/25.
//
#include "net/tcp/tcp_ring_buffer.h"


namespace mrpc {
    TCPRingBuffer::TCPRingBuffer(int buff_size) {
        RING_BUFFER_ASSERT(RING_BUFFER_IS_POWER_OF_TWO(buff_size)==1);
        m_buffer_mask = buff_size - 1;
        m_max_size = buff_size;
        m_read_idx = 0;
        m_write_idx = 0;
        m_buffer = std::make_unique<char[]>(buff_size);
    }

    TCPRingBuffer::~TCPRingBuffer() {
        m_buffer.reset();
    }

    void TCPRingBuffer::WriteRingBuffQueue(char data) {
        if (RingBuffIsFull()) {
            // 覆盖最老的字节，即读取位置向前增长
            m_read_idx = ((m_read_idx + 1) & m_buffer_mask);
        }
        m_buffer[m_write_idx] = data;
        m_write_idx = ((m_write_idx + 1) & m_buffer_mask);
    }

    void TCPRingBuffer::WriteRingBuffQueueArr(char *data, int size) {
        for (int i = 0; i < size; i++) {
            WriteRingBuffQueue(data[i]);
        }
    }

    int TCPRingBuffer::ReadRingBuffQueue(char *data) {
        if (RingBuffIsEmpty()) {
            // 为空不返回
            return 0;
        }
        *data = m_buffer[m_read_idx];
        m_read_idx = ((m_read_idx + 1) & m_buffer_mask);
        return 1;
    }

    int TCPRingBuffer::ReadRingBuffQueueArr(char *data, int size) {
        if (RingBuffIsEmpty()) {
            // 为空不返回
            return 0;
        }
        auto data_ptr = data;
        int cnt = 0;
        while ((cnt < size) && ReadRingBuffQueue(data_ptr)) {
            cnt++;
            data_ptr++;
        }
        return cnt;
    }

    std::vector<char> TCPRingBuffer::ReadRingBuffQueueArrVec(int size) {
        std::vector<char> data(size);
        // std::vector::data()返回一个指向内存数组的直接指针，该内存数组由向量内部用于存储其拥有的元素。
        auto cnt = ReadRingBuffQueueArr(data.data(), size);
        data.resize(cnt);
        return data;
    }

    void TCPRingBuffer::readFromBuffer(std::vector<char> &read_buff, int size) {
        auto data = ReadRingBuffQueueArrVec(size);
        read_buff.swap(data);
    }

    void TCPRingBuffer::writeToBuffer(char *buff, int size) {
        WriteRingBuffQueueArr(buff, size);
    }

    void TCPRingBuffer::writeToBuffer(const char *buff, int size) {
        WriteRingBuffQueueArr(const_cast<char *>(buff), size);
    }
}
