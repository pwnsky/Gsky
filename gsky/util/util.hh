#pragma once

#include <gsky/gsky.hh>
#include <sys/time.h>
#include <time.h>
#include <string>
namespace gsky {
namespace util {

std::string date_time();
std::string cat_file(const std::string &file_name);
std::string popen(const std::string &cmd);

}
}
