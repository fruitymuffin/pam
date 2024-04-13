#ifndef __STRINGTOOLS_H__
#define __STRINGTOOLS_H__

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace StringTools
{
    std::string macToString(int64_t mac)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 40) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 32) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 24) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << ((mac >> 8) & 0xff) << ":";
        ss << std::hex << std::setfill('0') << std::setw(2) << (mac & 0xff);

        return ss.str(); 
    }

    std::string ipToString(int64_t ip)
    {
        std::stringstream ss;
        ss << std::to_string((ip >> 24) & 0xff) << ":";
        ss << std::to_string((ip >> 16) & 0xff) << ":";
        ss << std::to_string((ip >> 8) & 0xff) << ":";
        ss << std::to_string(ip & 0xff);

        return ss.str();
    }

    std::string doubleToString(double val)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << val;
        
        return ss.str();
    }
}

#endif // __STRINGTOOLS_H__