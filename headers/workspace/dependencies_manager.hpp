#ifndef WORKSPACE_DEPENDENCIES_MANAGER
#define WORKSPACE_DEPENDENCIES_MANAGER

#include <set>
#include <string>

namespace workspace::dependencies_manager {
    void resolve_dependencies(std::set<std::string> dependencies);
}

#endif
