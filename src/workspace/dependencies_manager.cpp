#include "workspace/dependencies_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "workspace/project_config.hpp"
#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"
#include "commands.hpp"

namespace {
    using namespace workspace::dependencies_manager;

    namespace fs = std::filesystem;
    
    using namespace workspace::project_config;

    Project get_project_information(const fs::path& dependency) {
        fs::path current_path{ fs::current_path() };

        fs::current_path(current_path / dependency);

        Project project = convert_cfg_to_model();

        fs::current_path(current_path);

        return project;
    }

    Projects list_all_dependencies_available_locally() {
        Projects projects;

        for (const auto& project: fs::directory_iterator(fs::path("dependencies"))) {
            if (fs::is_directory(project)) {
                if (fs::is_directory(project)) {
                    projects.insert(get_project_information(project));
                }
            }
        }

        return projects;
    }

    Project fetch(const SurfaceDependency& dependency) {
        const std::string versioned_name{ dependency_to_string(dependency) };

        const std::string downloaded_file{ ".internals/tmp/" + versioned_name };
        const std::string extracted_directory{ downloaded_file + "__extracted" };
        const std::string dependency_path{ "dependencies/" + versioned_name };

        const std::string download_command{ "curl -L " + dependency.url + " -o " + downloaded_file };
        const std::string extract_command{ "tar -xzf " + downloaded_file + " -C " + extracted_directory + " --strip-components=1"};

        std::string error;

        Project project;

        std::cout << "[EXECUTE] " << download_command << "\n\n";

        if (system(download_command.c_str()) == 0) {
            std::cout << "\n[EXECUTE] " << extract_command << "\n\n";

            fs::create_directory(extracted_directory);
            
            if (system(extract_command.c_str()) == 0) {
                project = get_project_information(extracted_directory);

                if (project.name != dependency.name || project.version != dependency.version) {
                    error = "Project name/version mismatch with that provided in dependency declaration (while resolving '" + versioned_name + "')";
                } else {
                    fs::rename(extracted_directory, dependency_path);
                }
            } else {
                error = "Could not extract '" + versioned_name + "'";
            }
        } else {
            error = "Could not fetch '" + versioned_name + "' from '" + dependency.url + "'";
        }
        
        fs::remove(downloaded_file);

        if (fs::exists(extracted_directory)) {
            fs::remove_all(extracted_directory);
        }

        if (error != "") {
            throw std::runtime_error(error);
        } else {
            return project;
        }
    }

    SurfaceDependencies get_transitive_dependencies(const SurfaceDependency& dependency, const Projects& locally_stored_dependencies) {
        if (const auto project = locally_stored_dependencies.find(dependency); project != locally_stored_dependencies.end()) {
            return project->dependencies;
        } else {
            return fetch(dependency).dependencies;
        }
    }

    void linearise(const SurfaceDependencies& dependencies, std::map<SurfaceDependency, int, SurfaceDependencyComparator>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& dependency: dependencies) {
            const bool is_visited{ dependency_frequency.contains(dependency) };

            dependency_frequency[dependency]++;

            if (!is_visited) {
                SurfaceDependencies transitive_dependencies = get_transitive_dependencies(dependency, locally_stored_dependencies);
                linearise(transitive_dependencies, dependency_frequency, locally_stored_dependencies);
            }
        }
    }

    SurfaceDependencies resolve_versions(const std::map<SurfaceDependency, int, SurfaceDependencyComparator>& dependency_frequency) {
        std::map<std::string, SurfaceDependency> bucket;
        SurfaceDependencies resolved_dependencies;

        for (const auto& [dependency, count]: dependency_frequency) {
            const auto entry{ bucket.find(dependency.name) };

            if (entry == bucket.end()) {
                bucket[dependency.name] = dependency;
            } else {
                const auto existing_parsed_date{ workspace::util::parse_date(entry->second.version) };
                const auto candidate_parsed_date{ workspace::util::parse_date(dependency.version) };

                if (candidate_parsed_date > existing_parsed_date) {
                    entry->second = dependency;
                }
            }
        }

        for (const auto& [_, dependency]: bucket) {
            resolved_dependencies.insert(dependency);
        }

        return resolved_dependencies;
    }

    void remove_unnecessary_dependencies(const SurfaceDependencies& resolved_dependencies, const Projects& locally_stored_dependencies) {
        for (const auto& project: locally_stored_dependencies) {
            if (!resolved_dependencies.contains(SurfaceDependency{project.name, project.version})) {
                workspace::scaffold::remove_dependency(project.name, project.version);
            }
        }
    }

    void create_header_symlinks(const SurfaceDependencies& resolved_dependencies) {
        const fs::path project_root{ fs::current_path() };
        const fs::path symlink_path{ project_root / ".internals/dh_symlinks" };

        for (const auto& dependency: resolved_dependencies) {
            if (fs::exists(symlink_path / dependency.name) || fs::is_symlink(symlink_path / dependency.name)) {
                fs::remove(symlink_path / dependency.name);
            }
            
            #if defined(_WIN32) || defined(_WIN64)
            std::wstring cmd = L"mklink /J \"" +
                (symlink_path / dependency.name).make_preferred().wstring() + L"\\\" \"" +
                (project_root / L"dependencies" / dependency_to_string(dependency) / L"headers").make_preferred().wstring() + L"\"";
                
            system(std::string(cmd.begin(), cmd.end()).c_str());
            #else
            fs::create_directory_symlink(project_root / "dependencies" / dependency_to_string(dependency) / "headers", symlink_path / dependency.name);
            #endif
        }
    }

    int compile_uncompiled_dependencies(const SurfaceDependencies& resolved_dependencies) {
        fs::path project_root{ fs::current_path() };
        int compiled_dependencies_count{ 0 };

        for (const auto& dependency: resolved_dependencies) {
            if (!fs::exists(project_root / "build/dependencies" / dependency.name)) {
                const std::string versioned_name{ dependency_to_string(dependency) };

                fs::path dependency_root{ project_root / "dependencies" / versioned_name };
                fs::path dependency_build_root{ dependency_root / "build/binaries" };
                fs::path lifted_build_root{ project_root / "build/dependencies" / dependency.name };
                
                fs::current_path(dependency_root);

                std::cout << "[DEPENDENCY] " << versioned_name << "\n\n";

                commands::compile_project(true);

                std::cout << "\n";

                fs::current_path(project_root);

                fs::rename(dependency_build_root, lifted_build_root);

                workspace::scaffold::make_dependency_pristine(versioned_name);

                compiled_dependencies_count++;
            }
        }

        return compiled_dependencies_count;
    }

    void update_lockfile(const SurfaceDependencies& resolved_dependencies) {
        std::ofstream file_to_write("dependencies.lock");

        for (const auto& dependency: resolved_dependencies) {
            file_to_write << dependency_to_string(dependency, false) << "\n";
        }
        
        file_to_write.close();
    }
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(const SurfaceDependencies& dependencies) {
        Projects locally_stored_dependencies = list_all_dependencies_available_locally();

        std::map<SurfaceDependency, int, SurfaceDependencyComparator> dependency_frequency;

        linearise(dependencies, dependency_frequency, locally_stored_dependencies);

        const SurfaceDependencies resolved_dependencies{ resolve_versions(dependency_frequency) };

        remove_unnecessary_dependencies(resolved_dependencies, locally_stored_dependencies);
        create_header_symlinks(resolved_dependencies);

        const int compiled_dependencies_count{ compile_uncompiled_dependencies(resolved_dependencies) };

        if (compiled_dependencies_count == 0) {
            std::cout << "[INFO] All dependencies are up-to-date!\n";
        } else {
            update_lockfile(resolved_dependencies);

            std::cout << "[INFO] Compiled " << compiled_dependencies_count << " new dependencies.\n";
        }
    }
}
