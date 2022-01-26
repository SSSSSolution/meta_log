#include "meta_log/logger.h"
#include "meta_log/tool.h"
#include "meta_log/file_writer.h"
#include <stdarg.h>
#include <assert.h>
#include <atomic>
#include <stdexcept>

namespace meta_log {

class LoggerImpl
{
public:
    /////////////////////////////////////////////
    // member variables
    /////////////////////////////////////////////

    std::string log_dir;

    std::string start_date_time;

    std::string cur_log_file;

    int cur_log_file_seq;

    Level level = TRACE;

    bool output_to_console = true;

    bool output_to_file = true;

    size_t log_file_max_size = 5 * 1024 * 1024;

    uint32_t ignored_cosole_output_optional_flag = TIME | THREAD_ID | FILE_LINE | FUNC;

    FileWriter writer;

    bool output_to_file_async = false;



    /////////////////////////////////////////////
    // member func
    /////////////////////////////////////////////

    enum OutputType
    {
        CONSOLE,
        FILE,
    };

    // log format: [time(ns][tid:xxxx][module.filename.line:func][level]: msg
    void gen_log( OutputType output_type,
                  char *buf,
                  const char *level_str,
                  const char *module,
                  const char *file_line,
                  const char *func,
                  const char *fmt,
                  va_list args )
    {
        size_t idx = 0;
        size_t str_len;

        // [time(ns)]
        if (output_type == FILE ||
            !(ignored_cosole_output_optional_flag & TIME))
        {
            auto time_str = std::to_string(get_time_ns());
            str_len = time_str.length();
            buf[idx++] = '[';
            memcpy(buf + idx, time_str.c_str(), str_len);
            idx += str_len;
            buf[idx++] = ']';
        }

        // [tid:xxxxx]
        if (output_type == FILE ||
            !(ignored_cosole_output_optional_flag & THREAD_ID))
        {
            auto thread_str = get_thread_id_str();
            str_len = thread_str.length();
            memcpy(buf + idx, "[tid:", 5);
            idx += 5;
            memcpy(buf + idx, thread_str.c_str(), str_len);
            idx += str_len;
            buf[idx++] = ']';
        }

        // [module
        buf[idx++] = '[';
        str_len = strlen(module);
        memcpy(buf + idx, module, str_len);
        idx += str_len;

        // .filename.line
        if (output_type == FILE ||
            !(ignored_cosole_output_optional_flag & FILE_LINE))
        {
            const char *p_file_name = file_line;
            for (size_t i = strlen(file_line); i > 0; i--)
            {
                if (file_line[i - 1] == '\\')
                {
                    p_file_name = file_line + i;
                    break;
                }
            }

            buf[idx++] = '.';
            str_len = strlen(p_file_name);
            memcpy(buf + idx, p_file_name, str_len);
            idx += str_len;
        }

        // :func
        if (output_type == FILE ||
            !(ignored_cosole_output_optional_flag & FUNC))
        {
            buf[idx++] = ':';
            str_len = strlen(func);
            memcpy(buf + idx, func, str_len);
            idx += str_len;
        }

        // ]
        buf[idx++] = ']';

        // [level]
        buf[idx++] = '[';
        str_len = strlen(level_str);
        memcpy(buf + idx, level_str, str_len);
        idx += str_len;
        buf[idx++] = ']';

        // : msg
        buf[idx++] = ':';
        buf[idx++] = ' ';

        int ret = vsnprintf(buf + idx, META_LOG_BUFFER_SIZE - idx - 2, fmt, args);
        // ensure that the log has been completely written
        assert(ret >= 0 && ret < static_cast<int>(META_LOG_BUFFER_SIZE - idx -2));

        idx += static_cast<size_t>(ret);
        buf[idx++] = '\n';
        buf[idx++] = '\0';
    }
};

Logger *Logger::get_instance()
{
    static Logger logger;
    return &logger;
}

Logger::Logger()
    : impl(std::make_unique<LoggerImpl>())
{
    impl->start_date_time = get_date_str() + "_" + get_time_str();

    impl->writer.set_write_callback([&](int write_len){
        static int total_len = 0;
        total_len += write_len;
        if (total_len > static_cast<int>(impl->log_file_max_size))
        {
            std::string log_file = impl->log_dir + "/" + gen_log_file_name(++(impl->cur_log_file_seq));
            if (!impl->writer.redirect(log_file))
            {
                printf("can't create new log file");
            }
            total_len = 0;
        }
    });
}

void Logger::set_log_root_dir(const std::string &log_root_dir)
{
    impl->log_dir = log_root_dir + "/" + get_program_name() + "_" +
                    impl->start_date_time + "_log";
    if (!is_dir_exist(impl->log_dir.c_str()))
    {
        if (!create_path(impl->log_dir))
        {
            throw std::runtime_error("create log dir failed: " + log_root_dir);
        }
    }
}

void Logger::set_log_level(Level level)
{
    impl->level = level;
}

void Logger::set_output_to_console(bool b)
{
    impl->output_to_console = b;
}

void Logger::set_output_to_file(bool b)
{
    impl->output_to_file = b;
}

void Logger::set_async_output_to_file(bool b)
{
    impl->output_to_file_async = b;
}

void Logger::set_log_file_max_size(size_t max_size)
{
    impl->log_file_max_size = max_size;
}

void Logger::set_ignored_console_output_optional(uint32_t flag)
{
    impl->ignored_cosole_output_optional_flag = flag;
}

void Logger::log( Level level,
                  const char *level_str,
                  const char *module,
                  const char *file_line,
                  const char *func,
                  const char *fmt,
                  ... )
{
    if (level < impl->level)
    {
        return;
    }

    auto log_buf = std::make_shared<WriteBuf>();

    if (impl->output_to_console)
    {
        va_list args;
        va_start(args, fmt);
        impl->gen_log( LoggerImpl::CONSOLE,
                       log_buf->data,
                       level_str,
                       module,
                       file_line,
                       func,
                       fmt,
                       args );
        printf("%s", log_buf->data);
        va_end(args);
    }

    if (impl->output_to_file)
    {
        if (impl->log_dir.empty())
        {
            return;
        }

        if (!impl->writer.is_open())
        {
            std::string log_file = impl->log_dir + "/" + gen_log_file_name(impl->cur_log_file_seq);
            if (!impl->writer.open(log_file))
            {
                printf("Can't open file: %s", log_file.c_str());
                return;
            }
        }

        log_buf->len = strlen(log_buf->data);

        if (!impl->output_to_file_async)
        {
            impl->writer.sync_write(log_buf);
        }
        else
        {
            if (level >= ERROR)
            {
                impl->writer.sync_write(log_buf);
            }
            else
            {
                impl->writer.async_write(log_buf);
            }
        }
    }
}


}
























