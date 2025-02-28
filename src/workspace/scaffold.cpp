#include "workspace/scaffold.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "assets/scaffold_texts.hpp"
#include "workspace/modification_identifier.hpp"
#include "workspace/project_config.hpp"
#include "workspace/util.hpp"

namespace {
    using namespace workspace::scaffold;

    namespace fs = std::filesystem;
    
    using namespace assets::scaffold_texts;
    using std::cout;
    using std::endl;
    using std::ofstream;
    using std::string;
    using std::stringstream;
    using std::vector;

    string remove_raw_literal_indentations(const string& raw_literal) {
        string line, final_string;
        stringstream stream(raw_literal);
        vector<string> lines;

        while (getline(stream, line)) {
            lines.push_back(line);
        }

        lines.erase(lines.begin());
        lines.pop_back();

        for (auto& nline: lines) {
            final_string += (nline.size() > 4 ? nline.substr(4) : nline) + "\n";
        }

        return final_string;
    }

    string get_predefined_text_content(const workspace::project_config::Project& project, const string& file_name) {
        if (file_name.compare(".gitignore") == 0) {
            return remove_raw_literal_indentations(GITIGNORE);
        } else if (file_name.compare("docs/LICENSE.txt") == 0) {
            return remove_raw_literal_indentations(LICENSE_TXT);
        } else if (file_name.compare("docs/Roadmap.md") == 0) {
            return remove_raw_literal_indentations(ROADMAP_MD);
        } else if (file_name.compare("environments/.env.template") == 0) {
            return remove_raw_literal_indentations(ENV_TEMPLATE);
        } else if (file_name.starts_with("environments/") && file_name.ends_with(".env")) {
            return remove_raw_literal_indentations(ENV_FILE);
        } else if (file_name.compare("headers/cbt_tools/env_manager.hpp") == 0) {
            return remove_raw_literal_indentations(CBT_TOOLS_ENV_MANAGER_HPP);
        } else if (file_name.compare("headers/cbt_tools/test_harness.hpp") == 0) {
            return remove_raw_literal_indentations(CBT_TOOLS_TEST_HARNESS_HPP);
        } else if (file_name.compare("headers/cbt_tools/utils.hpp") == 0) {
            return remove_raw_literal_indentations(CBT_TOOLS_UTILS_HPP);
        } else if (file_name.compare("headers/forward_declarations.hpp") == 0) {
            const string text{ remove_raw_literal_indentations(FORWARD_DECLARATIONS_HPP) };

            const string with_scoped_namespace_start = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(text, START_SCOPE_R, "")
                : std::regex_replace(text, START_SCOPE_R, string("\n") + "namespace " + project.name + " {" + "\n");
            const string with_scoped_namespace_end = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "")
                : std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "\n}\n");

            return with_scoped_namespace_end;
        } else if (file_name.compare("src/cbt_tools/env_manager.cpp") == 0) {
            return remove_raw_literal_indentations(CBT_TOOLS_ENV_MANAGER_CPP);
        } else if (file_name.compare("src/cbt_tools/utils.cpp") == 0) {
            return remove_raw_literal_indentations(CBT_TOOLS_UTILS_CPP);
        } else if (file_name.ends_with(".hpp")) {
            const string text{ remove_raw_literal_indentations(SAMPLE_HPP) };
            const auto [stemmed_name, guard_name, namespace_name] = workspace::util::get_qualified_names(file_name);
            
            const string with_guard = std::regex_replace(text, GUARD_R, guard_name);
            const string with_import = std::regex_replace(with_guard, IMPORT_R, stemmed_name + ".hpp");
            
            const string with_scoped_namespace_start = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(with_guard, START_SCOPE_R, "")
                : std::regex_replace(with_guard, START_SCOPE_R, string("\n") + "namespace " + project.name + " {" + "\n");
            const string with_scoped_namespace_end = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "")
                : std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "\n}\n");
            
            const string final_text = std::regex_replace(with_scoped_namespace_end, NAMESPACE_R, namespace_name);
            
            return final_text;
        } else if (file_name.compare("src/main.cpp") == 0) {
            return remove_raw_literal_indentations(MAIN_CPP);
        } else if (file_name.starts_with("tests/unit_tests/")) {
            const string text{ remove_raw_literal_indentations(SAMPLE_TEST_CPP) };
            const auto [stemmed_name, _, namespace_name] = workspace::util::get_qualified_names(file_name);

            const string with_import = std::regex_replace(text, IMPORT_R, stemmed_name + ".cpp");
            
            #if defined(_WIN32) || defined(_WIN64)
            const string relative_path{ 
                std::regex_replace(
                    fs::relative(
                        "./src/" + stemmed_name + ".cpp",
                        "./" + fs::path(file_name).parent_path().string()
                    ).string(),
                    std::regex("\\\\"),
                    string("/")
                )
            };
            #else
            const string relative_path{ fs::relative(
                    "./src/" + stemmed_name + ".cpp",
                    "./" + fs::path(file_name).parent_path().string()
                ).string()
            };
            #endif
            
            const string with_relative_import = std::regex_replace(
                with_import, 
                RELATIVE_SRC_R, 
                relative_path
            );

            const string scoped_namespace_name = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? namespace_name
                : string(project.name + "::") + namespace_name;
                
            const string final_text = std::regex_replace(with_relative_import, NAMESPACE_R, scoped_namespace_name);
            
            return final_text;
        } else if (file_name.ends_with(".cpp")) {
            const string text{ remove_raw_literal_indentations(SAMPLE_CPP) };
            const auto [stemmed_name, _, namespace_name] = workspace::util::get_qualified_names(file_name);
            
            const string with_import = std::regex_replace(text, IMPORT_R, stemmed_name + ".hpp");

            const string with_scoped_namespace_start = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(with_import, START_SCOPE_R, "")
                : std::regex_replace(with_import, START_SCOPE_R, string("\n") + "namespace " + project.name + " {" + "\n");
            const string with_scoped_namespace_end = project.project_type == workspace::project_config::ProjectType::APPLICATION
                ? std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "")
                : std::regex_replace(with_scoped_namespace_start, END_SCOPE_R, "\n\n}");
            
            const string final_text = std::regex_replace(with_scoped_namespace_end, NAMESPACE_R, namespace_name);
            
            return final_text;
        } else if (file_name.compare("README.md") == 0) {
            const string text{ remove_raw_literal_indentations(README_MD) };
            const string with_project_name = std::regex_replace(text, PROJECT_NAME_R, project.name);

            return with_project_name;
        } else if (file_name.compare("project.cfg") == 0) {
            return workspace::project_config::convert_model_to_cfg(project);
        } else {
            return "";
        }
    }
}

namespace workspace::scaffold {
    void create_file(const std::optional<workspace::project_config::Project> project, const string& file_name, const bool verbose, const bool skip_root) {
        const string full_path = (!skip_root ? (project.value().name + "/") : "") + file_name;

        if (fs::exists(full_path)) {
            if (verbose) {
                cout << std::right << std::setw(8) <<  "SKIP " << full_path << endl;
            }
        } else {
            if (!fs::exists(full_path.substr(0, full_path.find_last_of("/")))) {
                create_directory("", full_path.substr(0, full_path.find_last_of("/")), true, verbose);
            }

            ofstream file_to_write(full_path);
            file_to_write << get_predefined_text_content(project.value(), file_name);
            file_to_write.close();

            if (verbose) {
                cout << std::right << std::setw(8) << "CREATE " << workspace::util::get_platform_formatted_filename(full_path) << endl;
            }
        }
    }

    bool create_directory(const string& project_name, const string& sub_directory, const bool multi_directory, const bool verbose) {
        string full_path = (project_name.length() != 0 ? (project_name + "/") : project_name)
            + sub_directory
            + (sub_directory.length() != 0 ? "/" : "");
        
        const bool result{ multi_directory ? fs::create_directories(full_path) : fs::create_directory(full_path) };

        if (result) {
            if (verbose) {
                cout << std::right << std::setw(8) << "DIR " << workspace::util::get_platform_formatted_filename(full_path) << endl;
            }
            return true;
        } else {
            return false;
        }
    }

    void create_build_tree_as_necessary() {
        if (!fs::exists("build/")) {
            workspace::scaffold::create_directory("", "build", false, false);
        }
        if (!fs::exists("build/binaries")) {
            workspace::scaffold::create_directory("", "build/binaries", false, false);
        }
        if (!fs::exists("build/test_binaries")) {
            workspace::scaffold::create_directory("", "build/test_binaries", false, false);
        }
        if (!fs::exists("build/test_binaries/unit_tests")) {
            workspace::scaffold::create_directory("", "build/test_binaries/unit_tests", false, false);
        }
    }

    void create_internals_tree_as_necessary() {
        if (!fs::exists(".internals/")) {
            workspace::scaffold::create_directory("", ".internals", false, false);
        }
        if (!fs::exists(".internals/tmp")) {
            workspace::scaffold::create_directory("", ".internals/tmp", false, false);
        }
        if (!fs::exists(".internals/timestamps.txt")) {
            workspace::scaffold::create_file(std::nullopt, ".internals/timestamps.txt", false, true);
        }
    }

    void create_dependencies_tree_as_necessary() {
        if (!fs::exists("dependencies/")) {
            workspace::scaffold::create_directory("", "dependencies", false, false);
        }
    }

    void purge_old_binaries(const string& path, const workspace::modification_identifier::SourceFiles& annotated_files) {
        if (path.compare("build/binaries/") != 0 && path.compare("build/test_binaries/unit_tests/") != 0) {
            throw std::domain_error("Unknown path '" + path + "' provided for purging. Only 'build/binaries/' and 'build/test_binaries/unit_tests/' allowed.");
        }

        std::vector<string> cpp_files{};

        const auto literal_length_of_src{ string("src/").length() };
        const auto literal_length_of_source_file_extension{ string(".cpp").length() };
        const auto literal_length_of_binary_file_extension{ string(".o").length() };

        for (auto const& source_file: annotated_files) {
            if (source_file.file_name.ends_with(".cpp")) {
                cpp_files.push_back(source_file.file_name.substr(literal_length_of_src, source_file.file_name.length() - literal_length_of_src - literal_length_of_source_file_extension));
            }
        }

        for (auto const& dir_entry: fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(dir_entry)) {
                const string binary_name{ workspace::util::get_platform_formatted_filename(dir_entry) };

                if (binary_name.ends_with(".o")) {
                    const string stemmed_name{ binary_name.substr(path.length(), binary_name.length() - path.length() - literal_length_of_binary_file_extension) };
                    bool adjacent_binary_found{ false };

                    for (auto const& file: cpp_files) {
                        if (stemmed_name.compare(file) == 0) {
                            adjacent_binary_found = true;
                            break;
                        }
                    }

                    if (!adjacent_binary_found) {
                        fs::remove(dir_entry);
                    }
                }
            }
        }
    }

    void exit_if_command_not_invoked_from_within_workspace() {
        if ((!fs::exists("project.cfg") || !fs::is_regular_file("project.cfg"))
            || (!fs::exists("headers/") || !fs::is_directory("headers"))
            || (!fs::exists("src/") || !fs::is_directory("src"))
            || (!fs::exists("tests/") || !fs::is_directory("tests"))) {
            throw std::runtime_error("Are you inside the project workspace?");
        }
    }
}
