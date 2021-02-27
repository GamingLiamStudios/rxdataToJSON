#pragma once

#include <any>
#include <fstream>
#include <vector>

#include <cmath>

#include "util/types.h"

#ifndef DOUBLE_INF
#define DOUBLE_INF  double(__DEC64_MAX__)
#define DOUBLE_NINF double(__DEC64_MIN__)
#endif

class Reader
{
public:
    Reader(std::ifstream &);
    std::any parse();

private:
    std::ifstream *file;

    std::vector<std::any>        object_cache;
    std::vector<std::vector<u8>> symbol_cache;
};
