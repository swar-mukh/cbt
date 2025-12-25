#include "workspace/dependencies_manager.hpp"

#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "workspace/project_config.hpp"
#include "workspace/scaffold.hpp"

namespace {
    using namespace workspace::dependencies_manager;

    namespace fs = std::filesystem;
    
    using namespace workspace::project_config;

    using Registry = std::map<std::string, std::set<std::string>>;

    const Registry registry = {
        { "lib1", {  "lib4", "lib5" } },
        { "lib2", {  "lib5" } },
        { "lib3", { "lib6", "lib7" } },
        { "lib4", {  } },
        { "lib5", {  } },
        { "lib6", {  } },
        { "lib7", { "lib6" } },
        { "lib8", { "lib9" } },
        { "lib9", { "lib8" } },
        { "lib10", { "lib8" } }
    };

    Project get_project_information(const std::string& dependency) {
        std::filesystem::path current_path = std::filesystem::current_path();

        std::filesystem::current_path(current_path / "dependencies" / dependency);

        Project project = convert_cfg_to_model();

        std::filesystem::current_path(current_path);

        return project;
    }

    Projects list_all_dependencies_available_locally() {
        Projects dependencies;

        for (const auto& entry: fs::directory_iterator(fs::path("dependencies"))) {
            if (fs::is_directory(entry)) {
                dependencies.insert(get_project_information(entry.path().filename().string()));
            }
        }

        return dependencies;
    }

    std::set<std::string> get_transitive_dependencies(const std::string& dependency, const Projects& locally_stored_dependencies) {
        if (const auto project = locally_stored_dependencies.find(dependency); project != locally_stored_dependencies.end()) {
            return project->dependencies;
        } else {
            // TODO: replace temporary simulation
            return registry.at(dependency);
        }
    }

    void linearise(const std::set<std::string>& dependencies, std::map<std::string, int>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& dependency: dependencies) {
            bool is_visited = dependency_frequency.contains(dependency);

            dependency_frequency[dependency]++;

            if (!is_visited) {
                std::set<std::string> transitive_dependencies = get_transitive_dependencies(dependency, locally_stored_dependencies);
                linearise(transitive_dependencies, dependency_frequency, locally_stored_dependencies);
            }
        }
    }

    void remove_unnecessary_dependencies(const std::map<std::string, int>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& dependency: locally_stored_dependencies) {
            if (!dependency_frequency.contains(dependency.name)) {
                workspace::scaffold::remove_dependency(dependency.name);
            }
        }
    }

    void compile_uncompiled_dependencies(const std::map<std::string, int>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& [dependency, _]: dependency_frequency) {
            if (!locally_stored_dependencies.contains(dependency)) {
                // TODO: compile them
            }
        }
    }
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(const std::set<std::string>& dependencies) {
        Projects locally_stored_dependencies = list_all_dependencies_available_locally();
        std::map<std::string, int> dependency_frequency;

        linearise(dependencies, dependency_frequency, locally_stored_dependencies);

        remove_unnecessary_dependencies(dependency_frequency, locally_stored_dependencies);
        compile_uncompiled_dependencies(dependency_frequency, locally_stored_dependencies);
    }
}
