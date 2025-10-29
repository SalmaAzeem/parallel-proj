#include "headers/SFMLWindowDrawer.hpp" 
#include <iostream>

int main() {
    try {
        SFMLWindowDrawer app(1280, 720, "Julia Sets Viewer");
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return -1;
    }
    return 0; 
}