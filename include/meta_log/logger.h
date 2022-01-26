#ifndef _META_LOG_LOGGER_H_
#define _META_LOG_LOGGER_H_

#include <memory>
#include <string>

#define META_LOG_BUFFER_SIZE 4096

//#define M_TRACE(fmt, ...)   META_LOG()

#define TO_STR_(X) #X
#define TO_STR(X) TO_STR_(X)

#define META_LOG(module, level, fmt, ...)       \
    do {                                        \
        meta_log::Logger::get_instance()->log(  \
            meta_log::Level::level,             \
            #level,                             \
            #module,                            \
            __FILE__"."TO_STR(__LINE__),        \
            __FUNCTION__,                       \
            fmt,                                \
            ## __VA_ARGS__                      \
        );                                      \
    } while(0);

namespace meta_log {

enum Level
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

enum ConsoleOutputOptional
{
    TIME = 0x01,
    THREAD_ID = 0x02,
    FILE_LINE = 0x04,
    FUNC = 0x08,
};

class LoggerImpl;
class Logger
{
public:
    static Logger *get_instance();

    void set_log_root_dir(const std::string &log_root_dir);

    void set_log_level(Level level);

    void set_output_to_console(bool);

    void set_output_to_file(bool);

    void set_async_output_to_file(bool);

    void set_log_file_max_size(size_t max_size);

    void set_ignored_console_output_optional(uint32_t flag);

    void log( Level level,
              const char *level_str,
              const char *module,
              const char *file_line,
              const char *func,
              const char *fmt,
              ...  );

private:
    Logger();

private:
    std::unique_ptr<LoggerImpl> impl;
};

}

#endif
