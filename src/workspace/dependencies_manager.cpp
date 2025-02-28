#include "workspace/dependencies_manager.hpp"

#include <iostream>
#include <set>
#include <string>

namespace {
    using namespace workspace::dependencies_manager;
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(std::set<std::string> dependencies) {
        for (const std::string &dependency: dependencies) {
            std::cout << "[DEPENDENCY] " << dependency << "\n";
        }
    }
}
