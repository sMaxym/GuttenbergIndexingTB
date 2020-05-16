#ifndef INDEXING_PARSING_H
#define INDEXING_PARSING_H

#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include "dcomp.h"

void parse(const std::string &text, std::map<std::string, size_t>& output);

#endif //INDEXING_PARSING_H
