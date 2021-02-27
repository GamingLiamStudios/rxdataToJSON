#pragma once

#include <string>
#include <any>
#include <unordered_map>

struct Object
{
    std::string name;

    std::unordered_map<std::string, std::any> list;
};
