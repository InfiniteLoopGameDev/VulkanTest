#include <exception>
#include <iostream>

#include "Application.h"

int main(int argv, char **args) {
    try {
        Application app;
        app.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
