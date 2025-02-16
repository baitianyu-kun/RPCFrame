#ifndef ROCKET_COMMON_LOG_H
#define ROCKET_COMMON_LOG_H

#include <string>
#include <queue>
#include <memory>
#include <semaphore.h>

#include "common/config.h"
#include "common/mutex.h"
#include "event/timer_fd_event.h"
#include "event/eventloop.h"

namespace mrpc {

#define DEBUGLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() && mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Debug) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushLog(mrpc::LogEvent(mrpc::LogLevel::Debug).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  } \

#define INFOLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Info) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushLog(mrpc::LogEvent(mrpc::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  } \

#define ERRORLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Error) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushLog(mrpc::LogEvent(mrpc::LogLevel::Error).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  }                        \

#define APPDEBUGLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Debug) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushAppLog(mrpc::LogEvent(mrpc::LogLevel::Debug).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  } \


#define APPINFOLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Info) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushAppLog(mrpc::LogEvent(mrpc::LogLevel::Info).toString() \
    + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  } \

#define APPERRORLOG(str, ...) \
  if (mrpc::Logger::GetGlobalLogger()->getLogLevel() <= mrpc::Error) \
  { \
    mrpc::Logger::GetGlobalLogger()->pushAppLog(mrpc::LogEvent(mrpc::LogLevel::Error).toString() \
      + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + mrpc::formatString(str, ##__VA_ARGS__) + "\n");\
  } \


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
        // 有一条就会写一条，所以肯定都会写入进去
        // 不用在退出时候遍历queue写入所有，因为在之前只要有数据就会写入，在退出前肯定能写入完
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
        explicit Logger(LogLevel level, int type = 1, bool is_server = true);

        ~Logger();

        void pushLog(const std::string &msg);

        void pushAppLog(const std::string &msg);

        void init_log_timer();

        //  同步 m_buffer 到 async_logger 的buffer队尾
        void syncLoop();

        void flush();

        AsyncLogger::async_logger_sptr_t_ getAsyncAppLogger() {
            return m_async_logger;
        }

        LogLevel getLogLevel() const {
            return m_set_level;
        }

    public:

        static std::unique_ptr<Logger> &GetGlobalLogger();

        static void InitGlobalLogger(int type = 1, bool is_server = true);

    private:
        LogLevel m_set_level;

        std::vector<std::string> m_buffer;
        std::vector<std::string> m_app_buffer;

        Mutex m_mutex;
        Mutex m_app_mutex;

        // RPC框架的信息
        AsyncLogger::async_logger_sptr_t_ m_async_logger;
        // 具体业务的信息
        AsyncLogger::async_logger_sptr_t_ m_async_app_logger;

        TimerEventInfo::ptr m_timer_event;
        int m_type{0}; // 为0就是只在控制台输出，1是输出到文件
        bool m_is_server{true};

        EventLoop::ptr m_event_loop{nullptr};
    };

    std::string LogLevelToString(LogLevel level);

    LogLevel StringToLogLevel(const std::string &log_level);

    // 将时间以及level等字符串进行聚合
    class LogEvent {
    public:
        explicit LogEvent(LogLevel level) : m_level(level) {}

        LogLevel getLogLevel() const {
            return m_level;
        }

        std::string toString();

    private:
        int32_t m_pid{0};  // 进程号
        int32_t m_thread_id{0};  // 线程号
        LogLevel m_level;     //日志级别
    };
}

#endif