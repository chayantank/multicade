#include <iostream>
#include <stdint.h>

template<typename T, typename U>
auto min_bug(T a, U b) -> decltype(a < b ? a : b)
{
    return a < b ? a : b;
}

int main() {
    int8_t start_y = -5;
    int8_t end_y = 10;
    int8_t lower_y = min_bug(start_y, end_y);
    std::cout << "min_bug: " << (int)lower_y << std::endl;
    return 0;
}
