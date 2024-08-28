//
// Created by baitianyu on 8/27/24.
//

#ifndef RPCFRAME_COROUTINE_H
#define RPCFRAME_COROUTINE_H

#include <functional>
#include <memory>
#include "coroutine/coctx.h"

namespace rocket {

    // 非对称有栈协程
    // 非对称代表均需要主协程来调度，对称是一个协程可以直接调度到另一个协程中
    // 有栈和无栈
    class Coroutine {
    public:
        using coroutine_sptr_t_ = std::shared_ptr<Coroutine>;

        static void Yield();

        static void Resume(Coroutine *coroutine);

        static Coroutine *GetCurrentCoroutine();

        static Coroutine *GetMainCoroutine();

        static bool IsMainCoroutine();

    public:
        Coroutine(int size, char *stack_ptr);

        Coroutine(int size, char *stack_ptr, std::function<void()> call_back);

        ~Coroutine();

        bool setCallBack(std::function<void()> call_back);

        void setIsInCoFunc(const bool v) {
            m_is_in_cofunc = v;
        }

        int getCorId() const {
            return m_cor_id;
        }


        bool getIsInCoFunc() const {
            return m_is_in_cofunc;
        }

        void setIndex(int index) {
            m_index = index;
        }

        int getIndex() {
            return m_index;
        }

        char *getStackPtr() {
            return m_stack_sp_ptr;
        }

        int getStackSize() {
            return m_stack_size;
        }

    private:
        Coroutine();

    public:
        std::function<void()> m_call_back;

    private:
        int m_cor_id{0}; // 协程id
        coctx m_coctx; // 协程寄存器上下文
        int m_stack_size{0}; // 协程申请堆空间的栈大小,单位: 字节
        // coroutine's stack memory space, you can malloc or mmap get some mermory to init this value
        char *m_stack_sp_ptr{nullptr};
        bool m_is_in_cofunc{false}; // 当call CoFunction的时候为true，false when CoFunction finished
        std::string m_msg_no;
        bool m_can_resume{true};
        int m_index{-1}; // index in coroutine pool

    };

}

#endif //RPCFRAME_COROUTINE_H
