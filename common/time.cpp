#include <chrono>

auto get_now()
{
    std::chrono::system_clock::time_point now;
    now = std::chrono::system_clock::now();
    return now;
}

double get_duration_ms(std::chrono::system_clock::time_point s,std::chrono::system_clock::time_point e){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(e-s).count();
}