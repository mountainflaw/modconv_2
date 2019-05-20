/* Test sanitizer */
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <cstdlib>

typedef signed char            s8;
typedef unsigned char          u8;
typedef signed short int       s16;
typedef unsigned short int     u16;
typedef signed int             s32;
typedef unsigned int           u32;
typedef signed long long int   s64;
typedef unsigned long long int u64;

typedef volatile u8   vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8   vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float  f32;
typedef double f64;

/** Acceptable characters */
std::string acceptableChars = "abcdefghijklmnopqrstuvwxyz0123456789",
               capitalChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

std::string sanitize_output(const std::string &input)
{
    std::string passOne = "",
                passTwo = "";

    for (u16 i = 0; i < input.length(); i++)
    {
        /* Convert C++ string to lowercase because goddamnit the standard library doesn't have this built in in a sane way. */
        for (u16 j = 0; j < 25; j++)
        {
            if (input.substr(i).compare(capitalChars.substr(j)) == 0)
                passOne += acceptableChars.substr(j);
            else
                passOne += input.substr(i);
        }

        /* Sanitize input by removing anything not a-z 0-9 with _. */
        for (u16 k = 0; k < 35; k++)
        {
            if (passOne.substr(i).compare(acceptableChars.substr(k)))
                passTwo += "_";
            else
                passTwo += input.substr(i);
        }
    }
    return passTwo;
}

int main()
{
    std::string dumb = "AVVGXabcdef!@#$%"; /* Should become avvgxabcdef_____*/
    std::cout << sanitize_output(dumb) << std::endl;
    return 0;
}
