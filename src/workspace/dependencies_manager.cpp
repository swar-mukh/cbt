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

    void linearise(const SurfaceDependencies& dependencies, std::map<std::string, int>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& dependency: dependencies) {
            const std::string versioned_name{ dependency_to_string(dependency) };
            const bool is_visited{ dependency_frequency.contains(versioned_name) };

            dependency_frequency[versioned_name]++;

            if (!is_visited) {
                SurfaceDependencies transitive_dependencies = get_transitive_dependencies(dependency, locally_stored_dependencies);
                linearise(transitive_dependencies, dependency_frequency, locally_stored_dependencies);
            }
        }
    }

    const std::map<std::string, int> resolve_version(const std::map<std::string, int>& dependency_frequency) {
        std::map<std::string, std::pair<std::string, int>> bucket;
        std::map<std::string, int> resolved_dependencies;

        for (const auto& [versioned_name, count]: dependency_frequency) {
            const auto [dependency_name, version] = workspace::util::get_key_value_pair_from_line(versioned_name, "@");
            
            const auto entry{ bucket.find(dependency_name) };

            if (entry == bucket.end()) {
                bucket[dependency_name] = std::make_pair(version, count);
            } else {
                const auto existing_parsed_date{ workspace::util::parse_date(entry->second.first) };
                const auto candidate_parsed_date{ workspace::util::parse_date(version) };

                if (candidate_parsed_date > existing_parsed_date) {
                    entry->second = std::make_pair(version, count);
                }
            }
        }

        for (const auto& [dependency_name, versioned_count]: bucket) {
            resolved_dependencies[dependency_name + "@" + versioned_count.first] = versioned_count.second;
        }

        return resolved_dependencies;
    }

    void remove_unnecessary_dependencies(const std::map<std::string, int>& dependency_frequency, const Projects& locally_stored_dependencies) {
        for (const auto& project: locally_stored_dependencies) {
            if (!dependency_frequency.contains(project.name + "@" + project.version)) {
                workspace::scaffold::remove_dependency(project.name, project.version);
            }
        }
    }

    void create_header_symlinks(const std::map<std::string, int>& resolved_dependencies) {
        const fs::path project_root{ fs::current_path() };
        const fs::path symlink_path{ project_root / ".internals/dh_symlinks" };

        for (const auto& [dependency, _]: resolved_dependencies) {
            const auto [dependency_name, version] = workspace::util::get_key_value_pair_from_line(dependency, "@");

            if (fs::exists(symlink_path / dependency_name) || fs::is_symlink(symlink_path / dependency_name)) {
                fs::remove(symlink_path / dependency_name);
            }
            
            #if defined(_WIN32) || defined(_WIN64)
            std::wstring cmd = L"mklink /J \"" +
                (symlink_path / dependency_name).make_preferred().wstring() + L"\\\" \"" +
                (project_root / L"dependencies" / dependency / L"headers").make_preferred().wstring() + L"\"";
                
            system(std::string(cmd.begin(), cmd.end()).c_str());
            #else
            fs::create_directory_symlink(project_root / "dependencies" / dependency / "headers", symlink_path / dependency_name);
            #endif
        }
    }

    void compile_uncompiled_dependencies(const std::map<std::string, int>& dependency_frequency) {
        fs::path project_root{ fs::current_path() };
        int compiled_dependencies_count{ 0 };

        for (const auto& [dependency, _]: dependency_frequency) {
            const auto [dependency_name, version] = workspace::util::get_key_value_pair_from_line(dependency, "@");
            
            if (!fs::exists(project_root / "build/dependencies" / dependency_name)) {
                fs::path dependency_root{ project_root / "dependencies" / dependency };
                fs::path dependency_build_root{ dependency_root / "build/binaries" };
                fs::path lifted_build_root{ project_root / "build/dependencies" / dependency_name };
                
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
            std::cout << "[INFO] All dependencies are up-to-date!\n";
        } else {
            std::cout << "[INFO] Compiled " << compiled_dependencies_count << " new dependencies.\n";
        }
    }
}

namespace workspace::dependencies_manager {
    void resolve_dependencies(const SurfaceDependencies& dependencies) {
        Projects locally_stored_dependencies = list_all_dependencies_available_locally();

        std::map<std::string, int> dependency_frequency;

        linearise(dependencies, dependency_frequency, locally_stored_dependencies);

        const std::map<std::string, int> resolved_dependencies{ resolve_version(dependency_frequency) };

        remove_unnecessary_dependencies(resolved_dependencies, locally_stored_dependencies);
        create_header_symlinks(resolved_dependencies);
        compile_uncompiled_dependencies(resolved_dependencies);
    }
}
