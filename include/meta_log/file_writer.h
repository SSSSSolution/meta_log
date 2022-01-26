#ifndef _META_LOG_FILE_WRITER_H_
#define _META_LOG_FILE_WRITER_H_

#include <string>
#include <cstdio>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <functional>
#include "meta_log/logger.h"

namespace meta_log
{

typedef std::function<void (size_t)> write_callback_t;

struct WriteBuf
{
    char data[META_LOG_BUFFER_SIZE];
    size_t len;
};

class FileWriter
{
public:
    FileWriter();
    ~FileWriter();

    bool is_open();

    bool open(const std::string &file_path);

    bool redirect(const std::string &file_path);

    void sync_write(std::shared_ptr<WriteBuf> buf);

    void async_write(std::shared_ptr<WriteBuf> buf);

    void set_write_callback(write_callback_t cb);

private:
    FILE *m_fp;
    int m_fd;

    std::mutex m_open_mutex;

    std::atomic<bool> m_stop_flag;
    std::mutex m_wait_mutex;
    std::condition_variable m_cv;
    std::unique_ptr<std::thread> m_async_write_thread;
    write_callback_t m_write_cb;

    std::mutex m_queue_mutex;
    std::shared_ptr<std::vector<std::shared_ptr<WriteBuf>>> m_pending_queue;
    std::shared_ptr<std::vector<std::shared_ptr<WriteBuf>>> m_logging_queue;
};

}

#endif
