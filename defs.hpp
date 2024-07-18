#pragma once

#include <string>

#include "json.hpp"

struct Struct1
{
    int i;
    std::string s;
};

void serializeElement(std::string &result, const Struct1 &t)
{
    result += O_O::encode(t.i, t.s);
}
void deserializeElement(const std::string &jsonData, int &index, Struct1 &t)
{
    O_O::decode(jsonData, index, t.i, t.s);
}

struct Struct2
{
    Struct1 struct1;
    std::string ss;
};

void serializeElement(std::string &result, const Struct2 &t)
{
    result += O_O::encode(t.struct1, t.ss);
}

void deserializeElement(const std::string &jsonData, int &index, Struct2 &t)
{
    O_O::decode(jsonData, index, t.struct1, t.ss);
}
