#include <filesystem>
#include <iostream>
#include <string>

#include <vector>
#include <unordered_map>

#include "reader/reader.h"

#include "reader/types/color.h"
#include "reader/types/object.h"
#include "reader/types/table.h"
#include "reader/types/tone.h"

namespace fs = std::filesystem;

void any_to_json(std::any &any, std::string &j)
{
    if (any.type() == typeid(std::string))
        j += "\"" + std::any_cast<std::string>(any) + "\"";
    else if (any.type() == typeid(bool))
        j += (std::any_cast<bool>(any) ? "true" : "false");
    else if (any.type() == typeid(NULL))
        j += "null";
    else if (any.type() == typeid(Object))
    {
        auto obj = std::any_cast<Object>(any);
        j += "{\"name\":\"" + obj.name + "\",\"values\":{";
        for (auto &pair : obj.list)
        {
            j += "\"" + pair.first + "\":";
            any_to_json(pair.second, j);
            j += ",";
        }
        if (obj.list.size() > 0) j.pop_back();
        j += "}}";
    }
    else if (any.type() == typeid(i32))
        j += std::to_string(std::any_cast<i32>(any));
    else if (any.type() == typeid(double))
        j += std::to_string(std::any_cast<double>(any));
    else if (any.type() == typeid(std::vector<u8>))
    {
        j += "[";
        for (auto &i : std::any_cast<std::vector<u8>>(any)) j += std::to_string(i) + ",";
        if (j.back() == '[') j += ",";
        j.back() = ']';
    }
    else if (any.type() == typeid(Color))
    {
        auto color = std::any_cast<Color>(any);
        j += "{\"Red\":" + std::to_string(color.red) + ",\"Green\":" + std::to_string(color.green) +
          ",\"Blue\":" + std::to_string(color.blue) + ",\"Alpha\":" + std::to_string(color.alpha) +
          "}";
    }
    else if (any.type() == typeid(Tone))
    {
        auto tone = std::any_cast<Tone>(any);
        j += "{\"Red\":" + std::to_string(tone.red) + ",\"Green\":" + std::to_string(tone.green) +
          ",\"Blue\":" + std::to_string(tone.blue) + ",\"Grey\":" + std::to_string(tone.grey) + "}";
    }
    else if (any.type() == typeid(Table))
    {
        auto table = std::any_cast<Table>(any);
        j += "{\"x\":" + std::to_string(table.x_size) + ",\"y\":" + std::to_string(table.y_size) +
          ",\"z\":" + std::to_string(table.z_size) +
          ",\"size\":" + std::to_string(table.data.size()) + ",\"data\":[";

        for (int z = 0; z < table.z_size; z++)
        {
            j += "[";
            for (int y = 0; y < table.y_size; y++)
            {
                j += "[";
                for (int x = 0; x < table.x_size; x++)
                    j +=
                      std::to_string(table.data[x + table.x_size * (y + table.y_size * z)]) + ",";
                j.pop_back();
                j += "],";
            }
            j.pop_back();
            j += "],";
        }
        if (table.z_size > 0) j.pop_back();
        j += "]}";
    }
    else if (any.type() == typeid(std::vector<std::any>))
    {
        auto list = std::any_cast<std::vector<std::any>>(any);
        j += list.size() == 0 ? "[," : "[";

        for (auto &a : list)
        {
            any_to_json(a, j);
            j += ",";
        }
        j.back() = ']';
    }
    else if (any.type() == typeid(std::unordered_map<i32, std::any>))
    {
        auto map = std::any_cast<std::unordered_map<i32, std::any>>(any);
        j += map.size() == 0 ? "{," : "{";
        for (auto &pair : map)
        {
            j += "\"" + std::to_string(pair.first) + "\":";
            any_to_json(pair.second, j);
            j += ",";
        }
        j.back() = '}';
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Please provide input!\n";
        return -1;
    }

    std::ifstream file;
    file.open(argv[1], std::ios::binary);

    // Verify Version
    char version[2];
    file.read(version, 2);
    if (version[0] != 4 || version[1] != 8)
    {
        std::string err =
          "Invalid Version: " + std::to_string(version[0]) + ", " + std::to_string(version[1]);
        std::__throw_invalid_argument(err.c_str());
    }

    std::cout << "Parsing\n";

    std::string j;
    Reader      reader(file);
    auto        parse = reader.parse();

    std::cout << "Converting to JSON\n";
    any_to_json(parse, j);
    std::cout << "Converted!\n";

    file.close();
    std::ofstream outfile;

    outfile.open(std::string(argv[1]) + ".json");
    outfile.write(j.data(), j.size());
    outfile.close();

    return 0;
}
