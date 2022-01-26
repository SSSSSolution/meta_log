#include "meta_log/file_writer.h"
#include <chrono>

#ifdef _MSC_VER
#include <io.h>
#endif

using namespace std::chrono_literals;

namespace meta_log
{

FileWriter::FileWriter()
    : m_fp(nullptr),
      m_fd(-1),
      m_stop_flag(false)
{
    m_pending_queue = std::make_shared<std::vector<std::shared_ptr<WriteBuf>>>();
    m_logging_queue = std::make_shared<std::vector<std::shared_ptr<WriteBuf>>>();

    m_async_write_thread = std::make_unique<std::thread>([&](){

        // write every 10ms
        while (!m_stop_flag.load())
        {
            {
                std::unique_lock<std::mutex> wait_lock(m_wait_mutex);
                m_cv.wait_for(wait_lock, 10ms);
            }

            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);
                std::swap(m_logging_queue, m_pending_queue);
            }

            for (auto buf : *m_logging_queue)
            {
                size_t ret = ::fwrite(buf->data, 1, buf->len, m_fp);
                if (m_write_cb)
                {
                    m_write_cb(ret);
                }
            }
            ::fflush(m_fp);
            m_logging_queue->clear();
        }

        // before exit, write remain buf to disk
        for (auto buf : *m_pending_queue)
        {
            ::fwrite(buf->data, 1, buf->len, m_fp);
        }
    });
}

FileWriter::~FileWriter()
{
    m_stop_flag.store(true);
    m_cv.notify_all();
    m_async_write_thread->join();

    if (m_fp != nullptr)
    {
        fclose(m_fp);
    }
}

bool FileWriter::is_open()
{
    std::unique_lock<std::mutex> lock(m_open_mutex);

    return m_fp != nullptr;
}

bool FileWriter::open(const std::string &file_path)
{
    std::unique_lock<std::mutex> lock(m_open_mutex);

    if (m_fp != nullptr)
    {
        return true;
    }

    ::fopen_s(&m_fp, file_path.c_str(), "a");
    if (m_fp != nullptr)
    {
        printf("open log file(%s) failed", file_path.c_str());
        return false;
    }

    #ifdef _MSC_VER
    m_fd = _fileno(m_fp);
    #endif
    return true;
}

bool FileWriter::redirect(const std::string &file_path)
{
    std::unique_lock<std::mutex> lock(m_open_mutex);

    if (m_fp == nullptr)
    {
        return false;
    }

    ::freopen_s(&m_fp, file_path.c_str(), "a", m_fp);
    if (m_fp == nullptr)
    {
        printf("reopen log file(%s) failed", file_path.c_str());
        return false;
    }
    return true;
}

void FileWriter::sync_write(std::shared_ptr<WriteBuf> buf)
{
    ::_write(m_fd, buf->data, static_cast<unsigned int>(buf->len));
}

void FileWriter::async_write(std::shared_ptr<WriteBuf> buf)
{
    std::unique_lock<std::mutex> lock(m_queue_mutex);

    m_pending_queue->push_back(buf);
}

void FileWriter::set_write_callback(write_callback_t cb)
{
    m_write_cb = cb;
}









}
