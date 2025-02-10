//
// Created by baitianyu on 2/7/25.
//

#ifndef RPCFRAME_ABSTRACT_TCP_BUFFER_H
#define RPCFRAME_ABSTRACT_TCP_BUFFER_H

#include <memory>
#include <vector>

#define MAX_TCP_BUFFER_SIZE 4096

namespace rocket {
    class TCPBuffer {
    public:
        using ptr = std::shared_ptr<TCPBuffer>;

        TCPBuffer() = default;

        virtual ~TCPBuffer() = default;

        virtual void readFromBuffer(std::vector<char> &read_buff, int size) = 0;

        virtual void writeToBuffer(char *buff, int size) = 0;

        virtual void writeToBuffer(const char *buff, int size) = 0;

        virtual int getReadIndex() const = 0;

        virtual int getWriteIndex() const = 0;

        virtual int readAbleSize() const = 0;

        virtual int writeAbleSize() const = 0;

    };
}

#endif //RPCFRAME_ABSTRACT_TCP_BUFFER_H
