#ifndef CFG_H
#define CFG_H

#include <string>
#include <boost/filesystem.hpp>

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <fstream>
#include <jsoncpp/json/json.h>

struct configuration_t
{
    std::string in_file, out_file1, out_file2;
    size_t threads;
};

configuration_t init(const int& argc, const char* argv[]);
configuration_t read_conf(const std::string& cf);
bool is_number(const std::string& s);

#endif // CFG_H
