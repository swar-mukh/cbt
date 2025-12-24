#include "workspace/dependencies_manager.hpp"

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "workspace/scaffold.hpp"

namespace {
    using namespace workspace::dependencies_manager;

    using Registry = std::map<std::string, std::set<std::string>>;

    bool is_available_locally(std::set<std::string>& filesystem, const std::string& dependency) {
            return filesystem.contains(dependency);
    }

    // TODO: replace temporary simulation
    std::set<std::string> fetch(std::string dependency, Registry& registry, std::set<std::string>& filesystem) {
        if (is_available_locally(filesystem, dependency)) {
            std::cout << "Local fetch called: " << dependency << std::endl;
            return registry[dependency];
        } else {
            std::cout << "Remote fetch called: " << dependency << std::endl;
            return registry[dependency];
        }
    }

    void linearise(std::set<std::string>& dependencies, Registry& registry, std::map<std::string, int>& dependency_frequency, std::set<std::string>& filesystem) {
        for (const auto& dependency: dependencies) {
            bool is_visited = dependency_frequency.find(dependency) != dependency_frequency.end();

            dependency_frequency[dependency]++;

            if (!is_visited) {
                std::set<std::string> transitive_dependencies = fetch(dependency, registry, filesystem);
                linearise(transitive_dependencies, registry, dependency_frequency, filesystem);
            }
        }
    }

    void remove_unnecessary_dependencies(std::map<std::string, int>& dependency_frequency, std::set<std::string>& locally_stored_dependencies) {
        for (const auto& dependency: locally_stored_dependencies) {
            if (dependency_frequency[dependency] == 0) {
                workspace::scaffold::remove_dependency(dependency);
            }
        }
    }

    void resolve_dependencies_internally(std::set<std::string>& primary_dependencies, Registry& registry, std::set<std::string>& filesystem) {
        std::map<std::string, int> dependency_frequency;
        linearise(primary_dependencies, registry, dependency_frequency, filesystem);

        for (const auto& [dependency, frequency] : dependency_frequency) {
            std::cout << dependency << " = " << frequency << "\n";
        }

        remove_unnecessary_dependencies(dependency_frequency, filesystem);
    }
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(std::set<std::string> dependencies) {
        Registry registry = {
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
        std::set<std::string> filesystem = { "lib3", "lib10", "lib1" };

        resolve_dependencies_internally(dependencies, registry, filesystem);
    }
}
