import std;

#include <cstdlib>

import vkt.Application;

int main([[maybe_unused]] int argv, [[maybe_unused]] char **args) {
    try {
        Application app;
        app.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
