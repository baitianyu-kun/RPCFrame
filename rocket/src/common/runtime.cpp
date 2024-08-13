//
// Created by baitianyu on 8/7/24.
//
#include "common/runtime.h"

namespace rocket {

    thread_local RunTime::run_time_sptr_t_ RunTime::t_run_time = std::make_shared<RunTime>();

    std::shared_ptr<RunTime> rocket::RunTime::GetRunTime() {
        return t_run_time;
    }

}



