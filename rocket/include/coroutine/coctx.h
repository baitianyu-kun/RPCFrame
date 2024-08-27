//
// Created by baitianyu on 8/27/24.
//

#ifndef RPCFRAME_COCTX_H
#define RPCFRAME_COCTX_H

namespace rocket{

    /*
     * 通用寄存器。共有rax、rbx、rcx、rdx、rsi、rdi、rbp、rsp、r8、r9、r10、r11、r12、r13、r14、r15这16个寄存器，
     * CPU对它们的用途没有做特殊规定，可以自定义其用途（其中rsp、rbp这两个寄存器有特殊用途）。
     * 程序计数寄存器（rip寄存器，也叫PC寄存器、IP寄存器）。用来存放下一条即将用来执行的指令的地址，它决定程序执行的流程。
     * 段寄存器（fs、gs寄存器）。用来实现线程本地存储（TLS），比如ADM64 Linux下Go语言和pthread线程库都用fs存储器来实现线程的TLS（本地存储）。
     */

    enum {
        kRBP = 6,   // rbp, bottom of stack 栈底指针
        kRDI = 7,   // rdi, first para when call function 第一个参数
        kRSI = 8,   // rsi, second para when call function 第二个参数
        kRETAddr = 9,   // the next excute cmd address, it will be assigned to rip 赋值给rip寄存器，下一条执行指令的地址
        kRSP = 13,   // rsp, top of stack 栈顶指针
    };

    struct coctx{
        //  regs是寄存器数组
        //  regs[kRETAddr] 为下一条指令的地址，一般我们创建协程初始时会传递一个函数的入口地址，
        //  regs[kRDI] 和 regs[kRSI] 是这个函数的第一、第二参数。切换协程时就会转而去执行这个函数的代码。
        void *regs[14];
    };

    // 使用了 asm 关键字将其映射到汇编代码中的 coctx_swap 函数。
    // coctx_swap.S: 这部分是汇编函数声明，global的意思是使函数在其他文件可见。
    extern "C"{
        extern void coctx_swap(coctx *, coctx *) asm("coctx_swap");
    }
}

#endif //RPCFRAME_COCTX_H
