#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <bitset>





// Parse the input string and create an array of 
// commands. If an error occured while parsing, return
// the leftover and stop.



int main(void)
{
    Command commands[MAX_NUM_COMMANDS];
    const char* p = " 0 10 3814 1 2443 123 2 s456245681 3967 4 12024 3967 4 1202 12024 3967 4 112024 3967 4 12024 3967 4 12024 3967 4 12024 3967 4 12024 3967 4 12024";
    char* end = nullptr;
    size_t idx;
    if (parse(p, &end, commands, idx))
    {
        std::cout << "Good parse!" << std::endl;
        for(int i = 0; i < idx; i++)
        {
            std::cout << commands[i] << std::endl;
        }
    }
    else
    {
        std::cout << "Bad parse!" << std::endl;
        printf("\nUnextracted leftover: '%s'\n\n", end);
    }

    return 0;
}