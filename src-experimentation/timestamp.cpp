#include <chrono>
#include <iostream>
#include <sstream>

std::string createFilename(unsigned int n, unsigned int n_max)
{
    std::stringstream ss;

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto micro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    ss << micro << "-" << n << "-" << n_max << ".png";
    
    return ss.str();
}

int main(void)
{
    int frames = 10;
    for(int i = 0; i < frames; i++)
    {
        std::cout << createFilename(i+1, frames) << std::endl;
    }

    return 0;
}
