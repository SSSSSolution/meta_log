#include "meta_log/tool.h"
#include <thread>

#ifdef _MSC_VER
#include <process.h>
#include <Windows.h>
#include <direct.h>
#endif

namespace meta_log
{

int get_pid()
{
#ifdef WIN32
    return ::_getpid();
#endif
}

std::string get_program_name()
{
#ifdef WIN32
    std::string program_full_path, program_name;
    program_full_path.resize(1024);
    do
    {
        unsigned int len = GetModuleFileNameA(nullptr, &program_full_path[0], static_cast< unsigned int >(program_full_path.size()));
        if (len < program_full_path.size())
        {
            program_full_path.resize(len);
            break;
        }

        program_full_path.resize(program_full_path.size() * 2);
    }
    while (program_full_path.size() < 65536);

    for (int i = static_cast<int>(program_full_path.length()) - 1; i >= 0; i--)
    {
        if (program_full_path.at(static_cast<size_t>(i)) == '\\')
        {
            program_name = std::string(program_full_path.c_str() + i + 1);
        }
    }
#endif

    return program_name;
}

std::string get_thread_id_str()
{
    std::hash<std::thread::id> tid_hash;
    auto tid = tid_hash(std::this_thread::get_id());
    return std::to_string(tid);
}

unsigned long long get_time_ns()
{
#ifdef _MSC_VER
    FILETIME file_time;
    GetSystemTimeAsFileTime(&file_time);
    ULARGE_INTEGER ul;
    ul.HighPart = file_time.dwHighDateTime;
    ul.LowPart = file_time.dwLowDateTime;
    return ul.QuadPart * 100;
#endif
    return 0;
}

void get_date(int &year, int &month, int &day)
{
#ifdef WIN32
    std::time_t tt;
    tt = time(nullptr);
    struct tm stm;
    localtime_s(&stm, &tt);
#endif
    year = stm.tm_year + 1900;
    month = stm.tm_mon + 1;
    day = stm.tm_mday;
}

void get_time(int &hour, int &min, int &sec)
{
#ifdef WIN32
    std::time_t tt;
    tt = time(nullptr);
    struct tm stm;
    localtime_s(&stm, &tt);
#endif
    hour = stm.tm_hour;
    min = stm.tm_min;
    sec = stm.tm_sec;
}

std::string get_date_str()
{
    int year, month, day;
    get_date(year, month, day);

    std::string year_str = std::to_string(year);

    std::string month_str = std::to_string(month);
    if (month_str.size() < 2)
    {
        month_str = "0" + month_str;
    }

    std::string day_str = std::to_string(day);
    if (day_str.size() < 2)
    {
        day_str = "0" + day_str;
    }

    return year_str + "_" + month_str + "_" + day_str;
}

std::string get_time_str()
{
    int hour, min, sec;
    get_time(hour, min, sec);

    std::string hour_str = std::to_string(hour);
    if (hour_str.size() < 2)
    {
        hour_str = "0" + hour_str;
    }

    std::string min_str = std::to_string(min);
    if (min_str.size() < 2)
    {
        min_str = "0" + min_str;
    }

    std::string sec_str = std::to_string(sec);
    if (sec_str.size() < 2)
    {
        sec_str = "0" + sec_str;
    }

    return hour_str + "_" + min_str + "_" + sec_str;
}

// log file name: appname_pid_date_time_seq.meta_log
std::string gen_log_file_name(int seq)
{
    std::string file_name = get_program_name() + "_" + std::to_string(get_pid()) + "_" +
                            get_date_str() + "_" + get_time_str() + "_" + std::to_string(seq) + ".meta_log";

    return file_name;
}

bool is_dir_exist(const std::string &dir)
{
#ifdef WIN32
    DWORD ftype = GetFileAttributesA(dir.c_str());
    if (ftype == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    if (ftype & FILE_ATTRIBUTE_DIRECTORY)
    {
        return true;
    }
    return false;
#endif
}

bool create_path(const std::string &path)
{
    std::string dir_path = path;
    for (size_t i = 1; i < path.size(); ++i)
    {
        if (dir_path[i] == '/')
        {
            dir_path[i] = '\0';
            if (!is_dir_exist(dir_path.c_str()))
            {
                #ifdef WIN32
                auto ret = _mkdir(dir_path.c_str());
                #endif
                if (ret != 0)
                {
                    if (errno != EEXIST)
                    {
                        printf("mkdir %s failed", dir_path.c_str());
                        return false;
                    }

                }
            }
            dir_path[i] = '/';
        }
    }
    return true;
}

}













