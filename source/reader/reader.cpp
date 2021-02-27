#include "reader.h"

#include <string>
#include <iostream>

#include "types/color.h"
#include "types/table.h"
#include "types/object.h"
#include "types/tone.h"

#include "util/demangle.h"

Reader::Reader(std::ifstream &file)
{
    this->file = &file;
}

std::any Reader::parse()
{
    int type = file->get();
    if (file->eof()) std::__throw_ios_failure("Unexpected EOF");

    auto ios_failure = [](std::string err) {
        std::__throw_ios_failure(err.c_str());
    };

    auto read_fixnum = [this, ios_failure]() -> i32 {
        u32 x;
        i8  c = file->get();

        if (c == 0) return 0;
        if (c > 0)
        {
            if (4 < c && c < 128) return c - 5;
            if (c > sizeof(i32)) ios_failure("Fixnum too big: " + std::to_string(c));

            x = 0;
            for (int i = 0; i < 4; i++) x = (i < c ? file->get() << 24 : 0) | (x >> 8);
        }
        else
        {
            if (-129 < c && c < -4) return c + 5;
            c = -c;
            if (c > sizeof(i32)) ios_failure("Fixnum too big: " + std::to_string(c));

            x              = -1;
            const u32 mask = ~(0xff << 24);
            for (int i = 0; i < 4; i++) x = (i < c ? file->get() << 24 : 0xff) | ((x >> 8) & mask);
        }

        return x;
    };

    switch (type)
    {
    case '@':    // Link
    {
        i32 index = read_fixnum();
        return object_cache.at(index - 1);
    }
    case 'I':    // IVar
    {
        auto name = parse();
        if (name.type() != typeid(std::string))
            ios_failure(std::string("Unsupported IVar Type: ") + demangle(name.type().name()));

        auto length = read_fixnum();

        object_cache.push_back(name);
        auto index = object_cache.size() - 1;

        for (i32 i = 0; i < length; i++)
        {
            std::string key;
            {
                auto akey = parse();
                if (akey.type() != typeid(std::vector<u8>))
                    ios_failure(
                      std::string("IVar key not symbol: ") + ::demangle(akey.type().name()));

                auto vec = std::any_cast<std::vector<u8> &>(akey);
                key      = std::string(vec.begin(), vec.end());
            }

            auto value = parse();

            // TODO: Do something with the character encoding
        }

        return object_cache.at(index);
    }
    case 'e':    // Extended
        ios_failure("Not yet Implemented: Extended");
    case 'C':    // UClass
        ios_failure("Not yet Implemented: UClass");
    case '0':    // Nil
        return NULL;
    case 'T':    // True
        return true;
    case 'F':    // False
        return false;
    case 'i':    // Fixnum
        return read_fixnum();
    case 'f':    // Float
    {
        auto str = std::string(read_fixnum(), '0');
        file->read(str.data(), str.length());

        double v;
        if (str.compare("nan") == 0) v = std::nan("");
        if (str.compare("inf") == 0) v = DOUBLE_INF;
        if (str.compare("-inf") == 0)
            v = DOUBLE_NINF;
        else
            v = ::strtod(str.c_str(), NULL);

        object_cache.push_back(v);
        return v;
    }
    case 'l':    // Bignum
    {
        int  sign = file->get();
        auto len  = read_fixnum();

        char data[len * 2];
        file->read(data, len * 2);

        // TODO

        ios_failure("Not yet Implemented: Bignum");
    }
    case '"':    // String
    {
        auto str = std::string(read_fixnum(), '0');
        file->read(str.data(), str.length());

        object_cache.push_back(str);
        return str;
    }
    case '/':    // RegExp
        ios_failure("Not yet Implemented: RegExp");
    case '[':    // Array
    {
        std::vector<std::any> arr;
        i32                   len = read_fixnum();
        arr.reserve(len);

        object_cache.push_back(arr);
        auto index = object_cache.size() - 1;

        for (long i = 0; i < len; i++)
            std::any_cast<std::vector<std::any> &>(object_cache.at(index)).push_back(parse());

        return object_cache.at(index);
    }
    case '{':    // Hash
    {
        std::unordered_map<i32, std::any> map;
        auto                              length = read_fixnum();

        object_cache.push_back(map);
        auto index = object_cache.size() - 1;

        for (i32 i = 0; i < length; i++)
        {
            auto key = parse();
            if (key.type() != typeid(i32))
                ios_failure(std::string("Hash key not Fixnum: ") + ::demangle(key.type().name()));

            auto value = parse();

            std::any_cast<std::unordered_map<i32, std::any> &>(object_cache.at(index))
              .insert(std::pair(std::any_cast<i32>(key), value));
        }

        return object_cache.at(index);
    }
    case '}':    // HashDef
        ios_failure("Not yet Implemented: HashDef");
    case 'S':    // Struct
        ios_failure("Not yet Implemented: Struct");
    case 'u':    // UserDef
    {
        std::string name;
        auto        pname = parse();
        if (pname.type() == typeid(std::vector<u8>))
        {
            auto vec = std::any_cast<std::vector<u8> &>(pname);
            name.reserve(vec.size());
            name.append(vec.begin(), vec.end());
        }
        else
            ios_failure(std::string("UserDef name not Symbol"));

        auto size = read_fixnum();

        if (name.compare("Color") == 0)
        {
            Color color;
            file->read((char *) &color.red, sizeof(double));
            file->read((char *) &color.green, sizeof(double));
            file->read((char *) &color.blue, sizeof(double));
            file->read((char *) &color.alpha, sizeof(double));

            object_cache.push_back(color);
            return color;
        }
        else if (name.compare("Table") == 0)
        {
            Table table;

            file->seekg(4, std::ios::cur);
            file->read((char *) &table.x_size, 4);
            file->read((char *) &table.y_size, 4);
            file->read((char *) &table.z_size, 4);

            i32 size;
            file->read((char *) &size, sizeof(i32));

            table.data.resize(size);
            for (i32 i = 0; i < size; i++) file->read((char *) &table.data[i], sizeof(i16));

            object_cache.push_back(table);
            return object_cache.back();
        }
        else if (name.compare("Tone") == 0)
        {
            Tone tone;

            file->read((char *) &tone.red, sizeof(double));
            file->read((char *) &tone.green, sizeof(double));
            file->read((char *) &tone.blue, sizeof(double));
            file->read((char *) &tone.grey, sizeof(double));

            object_cache.push_back(tone);
            return object_cache.back();
        }
        else
            ios_failure("Unsupported user defined class: " + name);
    }
    case 'U':    // User Marshal
        ios_failure("Not yet Implemented: User Marshal");
    case 'o':    // Object
    {
        Object o;
        auto   name = parse();
        if (name.type() == typeid(std::vector<u8>))
        {
            auto vec = std::any_cast<std::vector<u8> &>(name);
            o.name.reserve(vec.size());
            o.name.append(vec.begin(), vec.end());
        }
        else
            ios_failure(std::string("Object name not Symbol"));

        auto length = read_fixnum();
        o.list.reserve(length);

        object_cache.push_back(o);
        auto index = object_cache.size() - 1;

        for (i32 i = 0; i < length; i++)
        {
            std::string key;
            {
                auto akey = parse();
                if (akey.type() != typeid(std::vector<u8>))
                    ios_failure(
                      std::string("Object key not symbol: ") + ::demangle(akey.type().name()));

                auto vec = std::any_cast<std::vector<u8> &>(akey);
                key      = std::string(vec.begin(), vec.end());
            }

            if (key[0] != '@') ios_failure("Object Key not instance variable name");

            auto value = parse();

            std::any_cast<Object &>(object_cache.at(index)).list.insert(std::pair(key, value));
        }

        return object_cache.at(index);
    }
    case 'd':    // Data
        ios_failure("Not yet Implemented: Data");
    case 'M':    // Module Old
    case 'c':    // Class
    case 'm':    // Module
        ios_failure("Not yet Implemented: Module/Class");
    case ':':    // Symbol
    {
        std::vector<u8> arr;
        i32             len = read_fixnum();

        arr.resize(len);
        file->read((char *) arr.data(), len);

        symbol_cache.push_back(arr);
        return arr;
    }
    case ';':    // Symlink
    {
        i32 index = read_fixnum();
        return symbol_cache.at(index);
    }
    default: ios_failure("Unknown Value: " + std::to_string(type));
    }
    return NULL;
}