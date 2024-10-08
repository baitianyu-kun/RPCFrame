//
// Created by baitianyu on 7/19/24.
//

#ifndef RPCFRAME_TCP_BUFFER_H
#define RPCFRAME_TCP_BUFFER_H

#include <memory>
#include <vector>
#include <cstring>

#define MAX_TCP_BUFFER_SIZE 2048

namespace rocket {

    // 主要是封装了流的读取写入等，包括inbuffer和outbuffer
    // inbuffer: 在fd可读的情况，调用read从socket缓冲区中读取到数据，然后写入到inbuffer里面
    // outbuffer： 在fd可写的情况调用write把outbuffer中的数据都写入到socket中
    class TCPBuffer {
    public:
        using tcp_buffer_sptr_t_ = std::shared_ptr<TCPBuffer>;

        explicit TCPBuffer(int size);

        ~TCPBuffer();

        int readAbleSize();

        int writeAbleSize();

        void readFromBuffer(std::vector<char> &read_buff, int size);

        void writeToBuffer(const char *buff, int size);

        void resizeBuffer(int new_size);

        // read index左边的数据就不要了，整体都移动到开头，并设置read index为0，notion中回收调整
        void adjustBuffer();

        void moveReadIndex(int size);

        void moveWriteIndex(int size);

        int getReadIndex() const {
            return m_read_index;
        }

        int getWriteIndex() const {
            return m_write_index;
        }

        // 返回buff的引用，方便对其进行操作
        std::vector<char> &getRefBuffer() {
            return m_buffer;
        }

        std::vector<char> getCopiedBuffer() const {
            return m_buffer;
        };

        size_t getBufferSize() const {
            return m_buffer.size();
        }


    private:
        int m_read_index{0};
        int m_write_index{0};
        int m_size{0};
        std::vector<char> m_buffer;
    };
}


#endif //RPCFRAME_TCP_BUFFER_H
