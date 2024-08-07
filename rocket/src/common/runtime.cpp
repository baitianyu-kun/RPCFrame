//
// Created by baitianyu on 8/7/24.
//
#include "common/runtime.h"

namespace rocket {

    // ================================================================================================
    // 此时该变量是整个文件的全局变量，不是RunTime的成员变量，也可以这么写单例模式，前提是一个文件最好只有一个类的情况，
    // 否则别的类也能调用，不然还是应该给放到RunTime的成员里面，然后在类外初始化
    // static thread_local RunTime::run_time_sptr_t_ t_run_time = std::make_shared<RunTime>();
    // 写成封装性较好的单例模式的情况，在RunTime里面定义静态变量t_run_time
    // thread_local RunTime::run_time_sptr_t_ RunTime::t_run_time = std::make_shared<RunTime>();
    // ================================================================================================

    thread_local RunTime::run_time_sptr_t_ RunTime::t_run_time = std::make_shared<RunTime>();

    std::shared_ptr<RunTime> rocket::RunTime::GetRunTime() {
        return t_run_time;
    }

}



