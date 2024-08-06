#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>

#include "common/config.h"
#include "common/mutex.h"

namespace rocket {

    template<typename... Args>
    std::string formatString(const char *str, Args &&... args) {
        int size = snprintf(nullptr, 0, str, args...);
        std::string result;
        if (size > 0) {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }
        return result;
    }

#define DEBUGLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Debug) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::LogLevel::Debug)).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                                \
  } \


#define INFOLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Info) \
  { \
  rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::LogLevel::Info)).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
  rocket::Logger::GetGlobalLogger()->log();                                                                      \
  } \

#define ERRORLOG(str, ...) \
  if (rocket::Logger::GetGlobalLogger()->getLogLevel() <= rocket::Error) \
  { \
    rocket::Logger::GetGlobalLogger()->pushLog((rocket::LogEvent(rocket::LogLevel::Error)).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString(str, ##__VA_ARGS__) + "\n");\
    rocket::Logger::GetGlobalLogger()->log();                                                                                 \
  } \

    enum LogLevel {
        Unknown = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };

    class AsyncLogger {
    public:
        using async_logger_sptr_t_ = std::shared_ptr<AsyncLogger>;

        AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size);

        ~AsyncLogger();

        void stop();

        void flush(); // 刷新到磁盘

        void pushLogBuffer(std::vector<std::string> &vec);

    public:
        // 将 buffer 里面的全部数据打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程
        static void *Loop(void *arg);

    public:
        pthread_t m_thread{0};
    private:
        std::queue<std::vector<std::string>> m_buffer;
        std::string m_file_name;    // 日志输出文件名字
        std::string m_file_path;    // 日志输出路径
        int m_max_file_size{0};    // 日志单个文件最大大小, 单位为字节
        sem_t m_semaphore;

        // 1. 条件变量和互斥锁一样，都有静态动态两种创建方式，
        //    静态方式
        //    使用PTHREAD_COND_INITIALIZER常量，如下：
        //   　   pthread_cond_t cond = PTHREAD_COND_INITIALIZER
        //    动态方式
        //    调用pthread_cond_init()函数，第二个参数默认为null
        // 2. pthread_cond_wait()等待
        // 3. 激发条件有两种形式，pthread_cond_signal()激活一个等待该条件的线程，
        //    存在多个等待线程时按入队顺序激活其中一个；而pthread_cond_broadcast()则激活所有等待线程。
        pthread_cond_t m_condition;  // 条件变量

        Mutex m_mutex;
        std::string m_date;   // 当前打印日志的文件日期
        FILE *m_file_handler{NULL};   // 当前打开的日志文件句柄
        bool m_reopen_flag{false};
        int m_no{0};   // 日志文件序号
        bool m_stop_flag{false};
    };

    class Logger {
    public:
        typedef std::shared_ptr<Logger> s_ptr;

        Logger(LogLevel level) : m_set_level(level) {}

        void pushLog(const std::string &msg);

        void log();

        LogLevel getLogLevel() const {
            return m_set_level;
        }

    public:
        static Logger *GetGlobalLogger();

        static void InitGlobalLogger();

    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

        Mutex m_mutex;

    };

    std::string LogLevelToString(LogLevel level);

    LogLevel StringToLogLevel(const std::string &log_level);

    class LogEvent {
    public:

        LogEvent(LogLevel level) : m_level(level) {}

        std::string getFileName() const {
            return m_file_name;
        }

        LogLevel getLogLevel() const {
            return m_level;
        }

        std::string toString();

    private:
        std::string m_file_name;  // 文件名
        int32_t m_file_line;  // 行号
        int32_t m_pid;  // 进程号
        int32_t m_thread_id;  // 线程号

        LogLevel m_level;     //日志级别

    };
}

#endif