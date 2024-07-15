//
// Created by baitianyu on 7/14/24.
//
#include <sys/time.h>
#include <sstream>
#include <cstdio>
#include "common/log.h"
#include "common/util.h"
#include "common/config.h"




namespace rocket {

    static Logger *g_logger = NULL;

    Logger *Logger::GetGlobalLogger() {
        return g_logger;
    }


    void Logger::InitGlobalLogger() {

        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
        g_logger = new Logger(global_log_level);

    }


    std::string LogLevelToString(LogLevel level) {
        switch (level) {
            case Debug:
                return "DEBUG";

            case Info:
                return "INFO";

            case Error:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }


    LogLevel StringToLogLevel(const std::string &log_level) {
        if (log_level == "DEBUG") {
            return Debug;
        } else if (log_level == "INFO") {
            return Info;
        } else if (log_level == "ERROR") {
            return Error;
        } else {
            return Unknown;
        }
    }
}