#include "application.hpp"
#include <iostream>
#include <stdexcept>

int main(void) {
    VE::Application application{};
    
    try {
        application.run();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}