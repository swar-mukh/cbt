#include "workspace/dependencies_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "workspace/project_config.hpp"
#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"
#include "commands.hpp"

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
        fs::path current_path{ fs::current_path() };

        fs::current_path(current_path / "dependencies" / dependency);

        Project project = convert_cfg_to_model();

        fs::current_path(current_path);

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

    void create_header_symlinks(const std::map<std::string, int>& dependency_frequency) {
        const fs::path project_root{ fs::current_path() };
        const fs::path symlink_path{ project_root / ".internals/dh_symlinks" };

        for (const auto& [dependency, _]: dependency_frequency) {
            if (fs::exists(symlink_path / dependency)) {
                fs::remove(symlink_path / dependency);
            }
            
            #if defined(_WIN32) || defined(_WIN64)
            std::wstring cmd = L"mklink /J \"" +
                (symlink_path / dependency).make_preferred().wstring() + L"\\\" \"" +
                (project_root / L"dependencies" / dependency / L"headers").make_preferred().wstring() + L"\"";
                
            system(std::string(cmd.begin(), cmd.end()).c_str());
            #else
            fs::create_directory_symlink(project_root / "dependencies" / dependency / "headers", symlink_path / dependency);
            #endif
        }
    }

    void compile_uncompiled_dependencies(const std::map<std::string, int>& dependency_frequency) {
        fs::path project_root{ fs::current_path() };
        int compiled_dependencies_count{ 0 };

        for (const auto& [dependency, _]: dependency_frequency) {
            if (!fs::exists(project_root / "build/dependencies" / dependency)) {
                fs::path dependency_root{ project_root / "dependencies" / dependency };
                fs::path dependency_build_root{ dependency_root / "build/binaries" };
                fs::path lifted_build_root{ project_root / "build/dependencies" / dependency };
                
                fs::current_path(dependency_root);

                std::cout << "[DEPENDENCY] " << dependency << "\n\n";

                commands::compile_project(true);

                std::cout << "\n";

                fs::current_path(project_root);

                fs::rename(dependency_build_root, lifted_build_root);

                workspace::scaffold::make_dependency_pristine(dependency);

                compiled_dependencies_count++;
            }
        }

        if (compiled_dependencies_count == 0) {
            std::cout << "[INFO] All dependencies are up-to-date!";
        } else {
            std::cout << "[INFO] Compiled " << compiled_dependencies_count << " new dependencies.";
        }
    }
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(const std::set<std::string>& dependencies) {
        Projects locally_stored_dependencies = list_all_dependencies_available_locally();
        std::map<std::string, int> dependency_frequency;

        linearise(dependencies, dependency_frequency, locally_stored_dependencies);

        remove_unnecessary_dependencies(dependency_frequency, locally_stored_dependencies);
        create_header_symlinks(dependency_frequency);
        compile_uncompiled_dependencies(dependency_frequency);
    }
}
