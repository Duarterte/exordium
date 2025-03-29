#include <chrono>
#include <thread>
#include <iostream>

int main() {
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "tick" << std::endl;
    }
    return 0;
}