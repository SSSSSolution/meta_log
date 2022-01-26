#ifndef _META_LOG_TOOL_H_
#define _META_LOG_TOOL_H_

#include <string>

namespace meta_log
{

int get_pid();

std::string get_program_name();

std::string get_thread_id_str();

unsigned long long get_time_ns();

void get_date(int &year, int &month, int &day);

void get_time(int &hour, int &min, int &sec);

std::string get_date_str();

std::string get_time_str();

std::string gen_log_file_name(int seq);

bool is_dir_exist(const std::string &dir);

bool create_path(const std::string &path);













}



#endif
