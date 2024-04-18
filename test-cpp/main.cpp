#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <bitset>

// Writes to DAC a or b
// n    -> 0 for dac a, anything else for dac b
// vout -> output voltage 0 < vout < 4.096
void writeDACn(uint16_t n, float vout)
{
    // Constrain vout to 0 < vout < 4.096
    vout = vout < 0 ? 0 : vout > 4.096 ? 4.096 : vout;

    // Compute DAC data value to send
    uint16_t value = vout * 4096 / (2.048f * 2);

    // Add configuration bits for DAC selection (a/b) and normal operation
    value |= (n >= 0x01) << 15 | 0x01 << 13 | 0x01 << 12;

    // Send SPI
}

void shutdownDACn(uint16_t n)
{
    // Add configuration bits for DAC selection (a/b) and the shutdown bit
    uint16_t value = (n >= 0x01) << 15 & ~(0x01 << 12);
}

// These are the specific actions that 
enum Action
{
    WRITE_DAC_A,
    WRITE_DAC_B,
    LED_SHORT_ON,
    LED_SHORT_OFF,
    LED_ON,
    LED_OFF,
    LED_WHITE_ON,
    LED_WHITE_OFF,
    TRIGGER_HIGH,
    TRIGGER_LOW
};

struct Command
{
    int action;
    uint32_t time;
    float value;

    friend std::ostream& operator<< (std::ostream& stream, const Command& obj)
    {
        return stream << "[\n\tACT: " << obj.action << "\n\tTIME: " << obj.time << "\n\tVALUE: " << obj.value << "\n\n]";
    }
};

void parse(const char* c)
{
    printf("Parsing '%s':\n", c);

    Command cmd;
    uint32_t i;
    float f;

    uint8_t n_arg = 0;
    for (;;)
    {
        // Reset errno
        errno = 0;
        char *end;

        if ( (n_arg - 2) % 3 == 0 )
        {
            // should be a float arg
            f = strtof(c, &end);
            printf("Extracted '%.*s', strtof returned %f.", (int)(end-c), c, f);
        }
        else
        {
            i = strtoul(c, &end, 10);
            printf("Extracted '%.*s', strtol returned %ld.", (int)(end-c), c, i);
        }

        // No conversion
        if (c == end)
            break;
 
        const bool range_error = errno == ERANGE;

        c = end;
 
        if (range_error)
        {
            printf("\n --> Range error occurred.");
            break;
        }
 
        switch(n_arg)
        {
            case 0:
                cmd.action = int(i);
                n_arg++;
                break;
            case 1:
                cmd.time = i;
                n_arg++;
                break;
            case 2:
                cmd.value = f;
                n_arg = 0;

                //execute(cmd);
                std::cout << "\nEXECUTING!" << std::endl;
                break;
        }
        std::cout << std::endl;
    }
 
    printf("Unextracted leftover: '%s'\n\n", c);
}



int main(void)
{
    const char *p = " 0 10    1.22 3 100 0 4 110 0 1  2230   2.120ss";
    parse(p);
}
