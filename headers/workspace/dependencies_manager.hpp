#ifndef WORKSPACE_DEPENDENCIES_MANAGER
#define WORKSPACE_DEPENDENCIES_MANAGER

#include <set>
#include <string>

#include "workspace/project_config.hpp"

namespace workspace::dependencies_manager {
    void resolve_dependencies(const workspace::project_config::SurfaceDependencies& dependencies);
}

#endif
