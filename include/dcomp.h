#ifndef DCOMP_H
#define DCOMP_H

#include <string>
#include <stdexcept>
#include <archive.h>
#include <archive_entry.h>
#include "boost/filesystem.hpp"

#include <iostream>

constexpr size_t BUFFER_SIZE = 1024;

void decompress(const std::string &bin, std::string &res, const size_t& limit);

#endif // DCOMP_H
