#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> // Contains POSIX terminal control definitions
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <sys/ioctl.h>

namespace Tools
{
    inline std::string macToString(int64_t mac)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 40) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 32) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 24) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 8) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << (mac & 0xff);

        return ss.str(); 
    }

    inline std::string ipToString(int64_t ip)
    {
        std::stringstream ss;
        ss << std::to_string((ip >> 24) & 0xff) << ":";
        ss << std::to_string((ip >> 16) & 0xff) << ":";
        ss << std::to_string((ip >> 8) & 0xff) << ":";
        ss << std::to_string(ip & 0xff);

        return ss.str();
    }

    inline std::string doubleToString(double val)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << val;
        
        return ss.str();
    }

    inline int sendSerialString(std::string str)
    {
        // We wouldn't want to be opening/closing the serial port (file) repeatedly in a loop but since 
        // in practice this will only get called sporadically it makes things simpler.

        // Make sure str is terminated with "\r\n"
        size_t pos = 0;
        if(str.find("\r\n") == std::string::npos)
        {
            str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
            str.erase(std::remove(str.begin(), str.end(), '\r'), str.cend());
            str.append("\r\n");
        }

        // Open tty
        char byte;
        int fd = open("/dev/ttyUSB0", O_WRONLY);
        
        // Flush the serial i/o buffers.
        // Occasionally there will be some garbage which tends to throw off the MCU and image acquisition.
        ioctl(fd, TCFLSH, 2);

        struct termios tty;

        // Read in existing settings, and handle any error
        if(tcgetattr(fd, &tty) != 0) {
            printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
            return 1;
        }

        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
        tty.c_cflag |= CS8; // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

        // Set in/out baud rate to be 9600
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);

        // Save tty settings, also checking for error
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
            printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            return 1;
        }

        // write string
        write(fd, str.c_str(), str.size());

        close(fd);
        return 0;
    }

    // Approximate the irradiance based on the LED current.
    // These numbers are explained in section 4 of the report.
    static inline float irradiance(const float& i)
    {
        return 1818.4 * i + 135.26;
    }
}

#endif // __TOOLS_H__