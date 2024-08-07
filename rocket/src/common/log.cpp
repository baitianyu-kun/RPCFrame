#include <sys/time.h>
#include <sstream>
#include <cstdio>
#include <cassert>
#include "common/log.h"
#include "common/util.h"
#include "common/config.h"


#define TIME_ARRAY_SIZE 32

namespace rocket {

    static std::unique_ptr<Logger> g_logger = nullptr;

    std::unique_ptr<Logger> &Logger::GetGlobalLogger() {
        return g_logger;
    }

    void Logger::InitGlobalLogger(int type /*= 1*/, bool is_server /* true */) {
        LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->m_log_level);
        printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
        g_logger = std::move(std::unique_ptr<Logger>(new Logger(global_log_level, type, is_server)));
        g_logger->init_log_timer();
    }

    Logger::Logger(LogLevel level, int type /* = 1*/, bool is_server /* true */) : m_set_level(level), m_type(type) {
        if (m_type == 0) {
            return;
        }
        if (is_server) {
            m_async_logger = std::make_shared<AsyncLogger>(
                    Config::GetGlobalConfig()->m_log_file_name + "_server",
                    Config::GetGlobalConfig()->m_log_file_path,
                    Config::GetGlobalConfig()->m_log_max_file_size);
        } else {
            m_async_logger = std::make_shared<AsyncLogger>(
                    Config::GetGlobalConfig()->m_log_file_name + "_client",
                    Config::GetGlobalConfig()->m_log_file_path,
                    Config::GetGlobalConfig()->m_log_max_file_size);
        }
    }

    void Logger::init_log_timer() {
        // 初始化timer event，timer event负责定时同步log到async logger的队尾
        // timer event info中也有debug的log信息，如果放到构造函数中的话，就会出现此处需要创建timer event info，但是timer event info中
        // 又需要log创建，二者之间出现了相互依赖的问题，只能先创建logger，然后再去写方法init timer event
        if (m_type == 0) {
            return;
        }
        m_timer_event = std::make_shared<TimerEventInfo>(Config::GetGlobalConfig()->m_log_sync_interval,
                                                         true,
                                                         std::bind(&Logger::syncLoop, this));

        // ================================================OLD=======================================================
        // TCPServer是主线程，拿了一个event loop的unique ptr，
        // 然后每个io thread分别又拿了一个新的event loop的unique ptr(因为加了thread local关键字)
        // 此时再move的话，会导致tcp server中的event loop被move到这里，就出了问题
        // auto m_main_event_loop = std::move(std::unique_ptr<EventLoop>(EventLoop::GetCurrentEventLoop()));
        // 在logger创建的时候这个是个裸指针，TCPServer或者TCPClient创建后裸指针被TCPServer或TCPClient接管，
        // 此时下面的裸指针和TCPServer或TCPClient里面的unique ptr共同进行管理
        // 这里可能需要改一下，感觉还是有点问题
        // 同时，有可能在rpc channel中，还没来得及运行定时任务进行输出就结束进程了，造成日志出问题，得在结束的时候把所有日志都进行刷新
        // 还有就是崩溃了，出现异常退出时候得捕获信号，然后输出日志
        // ================================================OLD=======================================================
        // ================================================NEW=======================================================
        // 把event loop都改为shared ptr吧，因为不仅TCPServer或者TCPClient要用，可能有的其他地方也需要用到，例如log里面
        // ================================================NEW=======================================================
        m_event_loop = EventLoop::GetCurrentEventLoop(); // 这里也是避免上面出现的相互依赖的问题
        m_event_loop->addTimerEvent(m_timer_event);
    }

    void Logger::syncLoop() {
        // 同步 m_buffer 到 async_logger 的buffer队尾
        std::vector<std::string> tmp_vec;
        ScopeMutext<Mutex> lock(m_mutex);
        tmp_vec.swap(m_buffer);
        lock.unlock();
        if (!tmp_vec.empty()) {
            m_async_logger->pushLogBuffer(tmp_vec);
        }
        tmp_vec.clear();
    }

    void Logger::flush() {
        syncLoop();
        // 先暂停再输出
        m_async_logger->stop();
        m_async_logger->flush();
    }

    void Logger::pushLog(const std::string &msg) {
        if (m_type == 0) {
            printf("%s", (msg).c_str());
            return;
        }
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.emplace_back(msg);
        lock.unlock();
    }

    AsyncLogger::AsyncLogger(const std::string &file_name, const std::string &file_path, int max_size) : m_file_name(
            file_name), m_file_path(file_path), m_max_file_size(max_size) {
        // 初始化sem，后两个为0代表是当前线程的局部信号量，否则线程之间共享
        assert(sem_init(&m_semaphore, 0, 0) == 0);
        assert(pthread_create(&m_thread, nullptr, AsyncLogger::Loop, this) == 0);
        // wait，直到新的线程执行完创建logger，保证loop创建完后才认为是创建成功
        sem_wait(&m_semaphore);
    }

    AsyncLogger::~AsyncLogger() {

    }

    void AsyncLogger::stop() {
        m_stop_flag = true;
    }

    // 强制用fflush进行刷新
    void AsyncLogger::flush() {
        if (m_file_handler) {
            fflush(m_file_handler);
        }
    }

    void AsyncLogger::pushLogBuffer(std::vector<std::string> &vec) {
        ScopeMutext<Mutex> lock(m_mutex);
        m_buffer.emplace(vec);
        pthread_cond_signal(&m_condition); // 唤醒线程，去执行loop函数
        lock.unlock();
    }

    void *AsyncLogger::Loop(void *arg) {
        // 将buffer里面的数据全部打印到文件中，然后线程睡眠，直到有新的数据再重复这个过程
        auto logger = reinterpret_cast<AsyncLogger *>(arg);
        assert(pthread_cond_init(&logger->m_condition, nullptr) == 0);
        // 增加信号量，使主线程从AsyncLogger的构造函数中正常结束，主线程保证loop创建完后才认为是创建成功
        sem_post(&logger->m_semaphore);

        while (true) {
            ScopeMutext<Mutex> lock(logger->m_mutex);

            // 不能只检查一次条件，应该在每次唤醒的时候都进行检测

            // while 循环：while 循环确保在等待期间检查条件，以防止虚假唤醒（spurious wakeups）。
            // 在多线程编程中，线程可能会在没有明确收到信号的情况下醒来（虚假唤醒），
            // 因此需要在循环中检查条件，并在条件满足时才继续执行。

            // pthread_cond_signal在多处理器上可能同时唤醒多个线程，当你只能让一个线程处理某个任务时，
            // 其它被唤醒的线程就需要继续wait,while循环的意义就体现在这里了，
            // pthread_cond_signal()也可能唤醒多个线程，而如果你同时只允许一个线程访问的话，
            // 就必须要使用while来进行条件判断，以保证临界区内只有一个线程在处理
            while (logger->m_buffer.empty()) {
                pthread_cond_wait(&(logger->m_condition), logger->m_mutex.getMutex());
            }
            // 保存logger的头部
            std::vector<std::string> tmp;
            tmp.swap(logger->m_buffer.front());
            logger->m_buffer.pop();

            lock.unlock();

            // 时间
            timeval now;
            gettimeofday(&now, nullptr);

            struct tm now_time;
            localtime_r(&(now.tv_sec), &now_time);

            const char *format = "%Y%m%d";
            char date[TIME_ARRAY_SIZE];
            strftime(date, sizeof(date), format, &now_time);

            if (std::string(date) != logger->m_date) {
                logger->m_no = 0;
                logger->m_reopen_flag = true;
                logger->m_date = std::string(date);
            }
            // 文件handler已经关闭了的话需要重新打开
            if (logger->m_file_handler == nullptr) {
                logger->m_reopen_flag = true;
            }

            std::stringstream ss;;
            // <<就是往流里面写入东西，<<就是从流往外面输出东西，可以到int，string等常见类型，并可以互相进行类型转换
            // stringstream stream;
            // stream<<t;//向流中传值
            // out_type result;//这里存储转换结果
            // stream>>result;//向result中写入值
            ss << logger->m_file_path << logger->m_file_name << "_" << std::string(date) << "_log.";
            std::string log_file_name = ss.str() + std::to_string(logger->m_no);
            if (logger->m_reopen_flag) {
                if (logger->m_file_handler) {
                    fclose(logger->m_file_handler);
                }
                logger->m_file_handler = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false;
            }
            // ftell返回给定流 stream 的当前文件位置，即字节数
            if (ftell(logger->m_file_handler) > logger->m_max_file_size) {
                fclose(logger->m_file_handler);
                log_file_name = ss.str() + std::to_string(logger->m_no++);
                logger->m_file_handler = fopen(log_file_name.c_str(), "a");
                logger->m_reopen_flag = false;
            }
            // 进行输出
            for (const auto &item: tmp) {
                if (!item.empty()) {
                    fwrite(item.c_str(), 1, item.length(), logger->m_file_handler);
                }
            }
            fflush(logger->m_file_handler);
            if (logger->m_stop_flag) {
                // 进行退出
                return nullptr;
            }
        }
        return nullptr;
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

    std::string LogEvent::toString() {
        struct timeval now_time;

        gettimeofday(&now_time, nullptr);

        struct tm now_time_t;
        localtime_r(&(now_time.tv_sec), &now_time_t);

        char buf[128];
        strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
        std::string time_str(buf);
        int ms = now_time.tv_usec / 1000;
        time_str = time_str + "." + std::to_string(ms);

        m_pid = getPid();
        m_thread_id = getThreadId();

        std::stringstream ss;

        ss << "[" << LogLevelToString(m_level) << "]\t"
           << "[" << time_str << "]\t"
           << "[" << m_pid << ":" << m_thread_id << "]\t";

        return ss.str();
    }
}