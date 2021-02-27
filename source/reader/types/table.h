#pragma once

#include "util/types.h"

#include <vector>

struct Table
{
    i32 x_size;
    i32 y_size;
    i32 z_size;

    std::vector<i16> data;
};
