//
// Created by baitianyu on 8/7/24.
//

#ifndef RPCFRAME_RUNTIME_H
#define RPCFRAME_RUNTIME_H

#include <memory>


namespace rocket {
    // 1. void foo() {
    //      thread_local int staticVar;
    //    } 这种在blocks中等同于
    //    void foo() {
    //      static thread_local int staticVar;
    //    }
    // 2. thread_local作为类成员变量时必须是static的.
    //    thread_local作为类成员时也是对于每个thread分别分配了一个,而static则是全局一个.
    //    class MyClass {
    //     public:
    //     static thread_local int staticMemberVar;
    //    };
    //    thread_local int MyClass::staticMemberVar = 12;
    // 3. thread local在所有函数外面的话，等同于static。此时b不是RunTime的成员变量
    //    class RunTime {
    //     public:....
    //    };
    //    thread_local int b = 12;

    // 存储运行时候的关键信息，例如msg id和method name
    // 每个线程拥有自己的RunTime
    class RunTime {
    public:
        using run_time_sptr_t_ = std::shared_ptr<RunTime>;
    public:
        static thread_local run_time_sptr_t_ t_run_time;

        static run_time_sptr_t_ GetRunTime();

    public:
        std::string m_msg_id;
        std::string m_method_full_name;
    };
}

#endif //RPCFRAME_RUNTIME_H
