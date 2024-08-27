//
// Created by baitianyu on 8/27/24.
//

#include <atomic>
#include <cstring>
#include <cassert>
#include "coroutine/coroutine.h"
#include "common/log.h"

namespace rocket {

    static thread_local Coroutine *t_main_coroutine = nullptr;
    static thread_local Coroutine *t_cur_coroutine = nullptr;
    static std::atomic_int t_coroutine_count{0}; // 数量和id均为原子操作
    static std::atomic_int t_cur_coroutine_id{1};

    void CoFunction(Coroutine *coroutine) {
        // 调用函数
        if (coroutine != nullptr) {
            coroutine->setIsInCoFunc(true);
            coroutine->m_call_back();
            coroutine->setIsInCoFunc(false);
        }
        // here coroutine's callback function finished, that means coroutine's life is over. we should yiled main couroutine
        Coroutine::Yield();
    }

    // 主协程，并且私有，只能在这里面进行初始化
    // 主协程不需要申请堆空间，他直接使用的就是线程的栈。也不需要为主协程的m_coctx设置初值，
    // 因为后面协程切换的时候会保存当前寄存器上下文到主协程的m_coctx中的。
    // 其中 t_main_coroutine 和 t_cur_coroutine 是两个线程局部变量，分别指向主协程和当前正在执行的协程。
    // 为什么是线程局部变量，是因为每一个线程都需要一个主协程。
    Coroutine::Coroutine() {
        m_cor_id = 0; // 主协程的id为0
        t_coroutine_count++;
        memset(&m_coctx, 0, sizeof(m_coctx));
        t_cur_coroutine = this;
    }

    // 用户协程
    rocket::Coroutine::Coroutine(int stack_size, char *stack_ptr) : m_stack_size(stack_size),
                                                                    m_stack_sp_ptr(stack_ptr) {
        assert(stack_ptr);
        if (!t_main_coroutine) {
            t_main_coroutine = new Coroutine();
        }
        m_cor_id = t_cur_coroutine_id++;
        t_coroutine_count++;
    }

    // 用户协程
    rocket::Coroutine::Coroutine(int stack_size, char *stack_ptr, std::function<void()> call_back) : m_stack_size(
            stack_size), m_stack_sp_ptr(stack_ptr) {
        assert(m_stack_sp_ptr);
        if (!t_main_coroutine) {
            t_main_coroutine = new Coroutine();
        }
        setCallBack(call_back);
        m_cor_id = t_cur_coroutine_id++;
        t_coroutine_count++;
    }

    rocket::Coroutine::~Coroutine() {
        t_coroutine_count--;
    }

    bool rocket::Coroutine::setCallBack(std::function<void()> call_back) {
        if (this == t_main_coroutine) {
            ERRORLOG("main coroutine can't set call back");
            return false;
        }
        if (m_is_in_cofunc) {
            ERRORLOG("this coroutine is in CoFunction");
        }

        m_call_back = call_back;

        // 栈顶指针
        char *top = m_stack_sp_ptr + m_stack_size;
        // 字节对齐，-16LL后四位为0000，所以应该是对齐到8字节，x86的栈必须8字节对齐
        // 正数的补码就是其本身
        // 负数的补码是在其原码的基础上, 符号位不变, 其余各位取反, 最后+1. (即在反码的基础上+1)
        // 这段代码的作用是将top的地址向下舍入到最接近的16的倍数的位置，实现了16字节对齐。
        top = reinterpret_cast<char *>((reinterpret_cast<unsigned long>(top)) & -16LL);
        memset(&m_coctx, 0, sizeof(m_coctx));
        m_coctx.regs[kRSP] = top; // rsp栈顶指针，栈的指针从高地址走向低地址，所以这里的rsp是栈顶
        // m_coctx.regs[kRBP] = top;
        m_coctx.regs[kRETAddr] = reinterpret_cast<char *>(CoFunction); // 下一条要执行的指令，即协程的回调函数
        m_coctx.regs[kRDI] = reinterpret_cast<char *>(this);
        m_can_resume = true;
        return true;
    }

    Coroutine *Coroutine::GetCurrentCoroutine() {
        if (t_cur_coroutine == nullptr) {
            t_main_coroutine = new Coroutine();
            t_cur_coroutine = t_main_coroutine;
        }
        return t_cur_coroutine;
    }

    Coroutine *Coroutine::GetMainCoroutine() {
        if (t_main_coroutine) {
            return t_main_coroutine;
        }
        t_main_coroutine = new Coroutine();
        return t_main_coroutine;
    }

    bool Coroutine::IsMainCoroutine() {
        // 为空或者确实为main的时候为true
        if (t_main_coroutine == nullptr || t_cur_coroutine == t_main_coroutine) {
            return true;
        }
        return false;
    }

    // 从用户协程返回到主协程中
    void Coroutine::Yield() {
        if (t_main_coroutine == nullptr) {
            ERRORLOG("main coroutine is nullptr");
            return;
        }
        if (t_cur_coroutine == t_main_coroutine) {
            ERRORLOG("current coroutine is main coroutine");
            return;
        }
        auto cur_coroutine = t_cur_coroutine;
        t_cur_coroutine = t_main_coroutine;
        // 交换一下
        coctx_swap(&(cur_coroutine->m_coctx), &(t_main_coroutine->m_coctx));
    }

    // 从主协程中返回到用户协程
    void Coroutine::Resume(Coroutine *coroutine) {
        if (t_cur_coroutine != t_main_coroutine) {
            ERRORLOG("swap error, current coroutine must be main coroutine");
            return;
        }
        if (!t_main_coroutine) {
            ERRORLOG("main coroutine is nullptr");
            return;
        }
        if (!coroutine || !coroutine->m_can_resume) {
            ERRORLOG("pending coroutine is nullptr or can_resume is false");
            return;
        }
        if (t_cur_coroutine == coroutine) {
            DEBUGLOG("current coroutine is pending cor, needn't swap");
            return;
        }
        t_cur_coroutine = coroutine;
        coctx_swap(&(t_main_coroutine->m_coctx), &(coroutine->m_coctx));
    }
}



