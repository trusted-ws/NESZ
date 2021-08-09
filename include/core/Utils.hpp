#pragma once
#include "Types.hpp"

class Utils
{
    public:
        static bool cross(Word base, Byte offset);
        static Word offset(Word base, Byte offset);
        static Word low_byte(Word data);
        static Word high_byte(Word data);
};