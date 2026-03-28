#include "commands.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "gnu_toolchain.hpp"
#include "workspace/dependencies_manager.hpp"
#include "workspace/modification_identifier.hpp"
#include "workspace/project_config.hpp"
#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"

namespace {
    namespace fs = std::filesystem;
    
    using std::cout;
    using std::endl;

    using namespace workspace::project_config;
    using namespace workspace::scaffold;

    void create_project(const std::string& project_name, const ProjectType& project_type) {
        if (std::filesystem::exists(project_name)) {
            cout << "Directory '" << project_name << "' already exists!" << endl;
            return;
        }

        const auto [is_valid, reason_if_any] = workspace::util::is_valid_project_name(project_name);

        if (!is_valid) {
            cout << reason_if_any << endl;
            return;
        }

        const Project project = init(project_name, project_type);

        if (create_directory(project_name)) {
            create_file(project, ".gitignore");
            create_directory(project_name, ".internals");
            create_directory(project_name, ".internals/dh_symlinks");
            create_directory(project_name, ".internals/tmp");
            create_file(project, ".internals/timestamps.txt");
            create_directory(project_name, "build");
            create_directory(project_name, "build/binaries");
            create_directory(project_name, "build/dependencies");
            create_directory(project_name, "build/test_binaries");
            create_directory(project_name, "build/test_binaries/unit_tests");
            create_directory(project_name, "dependencies");
            create_directory(project_name, "docs");
            create_file(project, "docs/LICENSE.txt");
            create_file(project, "docs/Roadmap.md");
            create_directory(project_name, "environments");
            create_file(project, "environments/.env.template");
            create_file(project, "environments/local.env");
            create_file(project, "environments/production.env");
            create_file(project, "environments/test.env");
            create_directory(project_name, "headers");
            create_directory(project_name, "headers/c");
            create_file(project, "headers/c/linkage_demo.h");
            create_directory(project_name, "headers/cbt_tools");
            create_file(project, "headers/cbt_tools/env_manager.hpp");
            create_file(project, "headers/cbt_tools/test_harness.hpp");
            create_file(project, "headers/cbt_tools/utils.hpp");
            create_file(project, "headers/forward_declarations.hpp");
            create_file(project, "headers/sample.hpp");
            create_directory(project_name, "src");
            create_directory(project_name, "src/c");
            create_file(project, "src/c/linkage_demo.c");
            create_directory(project_name, "src/cbt_tools");
            create_file(project, "src/cbt_tools/env_manager.cpp");
            create_file(project, "src/cbt_tools/utils.cpp");
            create_file(project, ".dockerignore");
            create_file(project, "Dockerfile");

            if (project_type == ProjectType::APPLICATION) {
                create_file(project, "src/main.cpp");
            }
            
            create_file(project, "src/sample.cpp");
            create_directory(project_name, "tests");
            create_directory(project_name, "tests/unit_tests");
            create_directory(project_name, "tests/unit_tests/c");
            create_file(project, "tests/unit_tests/c/linkage_demo.cpp");
            create_file(project, "tests/unit_tests/sample.cpp");
            create_file(project, "README.md");
            
            create_file(project, "project.cfg");

            cout << endl << "Project '" << project_name << "' created" << endl;
        } else {
            cout << "Could not create project '" << project_name << "'!" << endl;
        }
    }

    void list_directories_containing_binaries(std::set<string>& directories_containing_binaries, const string& build_path) {
        const string BUILD_PATH{ workspace::util::get_platform_formatted_filename(fs::path(build_path)) };

        for (auto dir_entry = fs::recursive_directory_iterator("build"); dir_entry != fs::recursive_directory_iterator(); ++dir_entry) {
            const string normalised_path{ workspace::util::get_platform_formatted_filename(dir_entry->path().string()) };

            if (!normalised_path.starts_with(BUILD_PATH)) {
                dir_entry.disable_recursion_pending();
            }
            
            if (fs::is_directory(*dir_entry)) {
                const int files_count = std::ranges::count_if(
                    fs::directory_iterator(dir_entry->path()),
                    [](const auto& file){ return file.is_regular_file(); }
                );

                if (files_count != 0) {
                    directories_containing_binaries.insert(normalised_path);
                }
            }
        }
    }

    bool are_dependencies_unresolved(const SurfaceDependencies& dependencies) {
        const auto it = std::ranges::find_if_not(dependencies,
            [](const auto& dependency){ return fs::exists("build/dependencies/" + dependency.name); }
        );

        if (it != dependencies.end()) {
            cout << "Dependency '" << it->name << "' not resolved! Run 'cbt resolve-dependencies' first." << endl;
            return true;
        }

        return false;
    }
}

namespace commands {
    namespace fs = std::filesystem;

    using std::cout;
    using std::endl;
    using std::string;

    using namespace workspace::project_config;
    
    void create_application(const string& project_name) {
        create_project(project_name, ProjectType::APPLICATION);
    }

    void create_library(const string& project_name) {
        create_project(project_name, ProjectType::LIBRARY);
    }
    
    void create_file(const string& file_name, const bool requires_c_linkage) {
        const auto [is_valid, reason_if_any] = workspace::util::is_valid_file_name(file_name, requires_c_linkage);

        if (!is_valid) {
            cout << reason_if_any << endl;
            return;
        }

        const Project project = convert_cfg_to_model(); 

        const bool create_only_header_file{ file_name.starts_with(requires_c_linkage ? "headers/c/" : "headers/") };

        const string header_file_extension{ requires_c_linkage ? ".h" : ".hpp" };

        if (create_only_header_file) {
            workspace::scaffold::create_file(project, file_name + header_file_extension, true, true);
        } else {
            const string implementation_file_extension{ requires_c_linkage ? ".c" : ".cpp" };
            
            const string header_prefix{ requires_c_linkage ? "headers/c/" : "headers/" };
            const string src_prefix{ requires_c_linkage ? "src/c/" : "src/" };
            const string test_prefix{ requires_c_linkage ? "tests/unit_tests/c/" : "tests/unit_tests/" };

            workspace::scaffold::create_file(project, header_prefix + file_name + header_file_extension, true, true);
            workspace::scaffold::create_file(project, src_prefix + file_name + implementation_file_extension, true, true);
            workspace::scaffold::create_file(project, test_prefix + file_name + ".cpp", true, true);
        }
    }

    void resolve_dependencies() {
        const Project project = convert_cfg_to_model();

        workspace::scaffold::create_working_tree_as_necessary();

        workspace::dependencies_manager::resolve_dependencies(project);
    }

    void compile_project(const bool compile_as_dependency) {
        workspace::scaffold::create_working_tree_as_necessary();

        const Project project = convert_cfg_to_model();

        if (!compile_as_dependency && are_dependencies_unresolved(project.dependencies)) {
            return;
        }

        const int literal_length_of_headers = string("headers/").length();
        const int literal_length_of_src = string("src/").length();

        workspace::modification_identifier::SourceFiles annotated_files = workspace::modification_identifier::list_all_files_annotated(project, compile_as_dependency);
        const int number_of_cpp_files_to_compile = std::ranges::count_if(
            annotated_files,
            [](const auto& file){ return (file.file_name.ends_with(".c") || file.file_name.ends_with(".cpp")) && file.affected; }
        );

        if (number_of_cpp_files_to_compile == 0) {
            cout << "[INFO] Nothing to compile: all files are up-to-date!" << endl;
            return;
        }

        cout << "[INFO] Number of file(s) to compile: " << number_of_cpp_files_to_compile << endl << endl;

        for (auto const& dir_entry: fs::recursive_directory_iterator("headers")) {
            if (fs::is_directory(dir_entry)) {
                const string directory = dir_entry.path().string();
                const string directory_under_check = string("build/binaries/" + directory.substr(literal_length_of_headers));

                if (!fs::exists(directory_under_check)) {
                    workspace::scaffold::create_directory(string("."), directory_under_check, false, false);
                }
            }
        }

        cout << "[COMMAND] " << gnu_toolchain::get_compilation_command(project, compile_as_dependency) << endl << endl;

        int files_succesfully_compiled_count{ 0 };

        for (auto& file: annotated_files) {
            if ((file.file_name.ends_with(".c") || file.file_name.ends_with(".cpp")) && file.affected) {
                const bool is_c_file{ file.file_name.ends_with(".c") };
                const int literal_length_of_extension = string(is_c_file ? ".c" : ".cpp").length();

                const string stemmed_file = file.file_name.substr(
                    literal_length_of_src,
                    file.file_name.length() - (literal_length_of_src + literal_length_of_extension)
                );
                const string header_extension{ is_c_file ? ".h" : ".hpp" };

                if (stemmed_file.compare("main") != 0 && !fs::exists("headers/" + stemmed_file + header_extension)) {
                    cout << "SKIP " << ("headers/" + stemmed_file + header_extension) << " (No corresponding implementation file found!)" << endl;
                } else {
                    file.compilation_start_timestamp = workspace::modification_identifier::get_current_fileclock_timestamp();

                    const int result = gnu_toolchain::compile_file(project, file.file_name, stemmed_file, compile_as_dependency);

                    file.compilation_end_timestamp = workspace::modification_identifier::get_current_fileclock_timestamp();
                    file.was_successful = (result == 0);

                    cout << "[COMPILE]" << std::left << std::setw(6) << (file.was_successful ? "[OK]" : "[NOK]") << file.file_name <<  endl;

                    if (file.was_successful) {
                        ++files_succesfully_compiled_count;
                    }
                }
            }
        }

        cout << endl << "[INFO] File(s) successfully compiled: " << files_succesfully_compiled_count << " out of " << number_of_cpp_files_to_compile << endl;

        workspace::scaffold::purge_old_binaries("build/binaries/", annotated_files);
        workspace::modification_identifier::persist_annotations(annotated_files);
    }

    void clear_build() {
        if (fs::remove_all(fs::current_path() / "build")) {
            cout << std::right << std::setw(8) << "RECREATE " << "build/" << endl;
        }

        if (fs::remove_all(fs::current_path() / ".internals")) {
            cout << std::right << std::setw(8) << "RECREATE " << ".internals/" << endl;
        }

        workspace::scaffold::create_working_tree_as_necessary();
    }

    void build_project() {
        const Project project = convert_cfg_to_model();

        if (project.project_type == ProjectType::LIBRARY) {
            cout << "This option is only available for applications." << endl;
            return;
        }

        if (!fs::exists("build/")) {
            cout << "Directory 'build/' does not exist! Run 'cbt compile-project' first." << endl;
            return;
        }
        
        if (!fs::exists("build/binaries")) {
            cout << "Directory 'build/binaries/' does not exist! Run 'cbt compile-project' first." << endl;
            return;
        }

        if (are_dependencies_unresolved(project.dependencies)) {
            return;
        }

        std::set<string> non_empty_directories;
        const string SEPARATOR{ fs::path::preferred_separator };

        list_directories_containing_binaries(non_empty_directories, "build/binaries");
        list_directories_containing_binaries(non_empty_directories, "build/dependencies");
        
        if (non_empty_directories.empty()) {
            cout << "No binaries present! Run 'cbt compile-project' first." << endl;
            return;
        }

        std::vector<string> directories_containing_binaries(non_empty_directories.begin(), non_empty_directories.end());

        #if defined(_WIN32) || defined(_WIN64)
        const string BINARY_NAME{ project.name + ".exe" };
        #else
        const string BINARY_NAME{ project.name };
        #endif

        const int result = gnu_toolchain::perform_linking(project, directories_containing_binaries, string("build") + SEPARATOR + BINARY_NAME);

        cout << "[BUILD]" << std::left << std::setw(6) << (result == 0 ? "[OK]" : "[NOK]") << workspace::util::get_platform_formatted_filename("build/" + BINARY_NAME) << endl;
    }

    void run_unit_tests() {
        workspace::scaffold::create_working_tree_as_necessary();

        const Project project = convert_cfg_to_model();

        if (are_dependencies_unresolved(project.dependencies)) {
            return;
        }

        const workspace::modification_identifier::RawDependencyTree tree = workspace::modification_identifier::get_files_to_test(project);

        if (tree.empty()) {
            cout << "[INFO] Nothing to compile or test. All files are up-to-date!" << endl;
            return;
        }

        cout << "[INFO] Number of file(s) to compile: " << tree.size() << endl << endl;
        
        #if defined(_WIN32) || defined(_WIN64)
        const string EXTENSION{ ".exe" };
        #else
        const string EXTENSION{ "" };
        #endif

        const fs::path harness{ "headers/cbt_tools/test_harness.hpp" };

        std::vector<fs::path> binaries_to_execute{};
        const size_t literal_length_of_headers{ std::string("headers/").length() };
        const size_t literal_length_of_dependencies{ std::string(".internals/dh_symlinks/").length() };

        cout << "[COMMAND] " << gnu_toolchain::get_test_execution_command(project, EXTENSION) << endl << endl;

        for (auto const& [file, dependencies]: tree) {
            const bool is_c_file{ file.starts_with("tests/unit_tests/c/") };

            std::vector<string> files_to_link{ file };
            const fs::path scoped_directory_of_file{ fs::relative(fs::path{ file }.parent_path(), "tests/unit_tests") };
            const fs::path build_directory_under_check{ "build/test_binaries/unit_tests" / scoped_directory_of_file };

            if (!fs::exists(build_directory_under_check)) {
                workspace::scaffold::create_directory(string("."), build_directory_under_check.string(), true, false);
            }

            const string header_extension{ is_c_file ? ".h" : ".hpp" };
            const fs::path corresponding_header_file{ fs::path("headers" / scoped_directory_of_file / fs::path(file).stem().replace_extension(header_extension)) };

            for (auto const& dependency: dependencies) {
                if (!fs::equivalent(corresponding_header_file, dependency) && !fs::equivalent(dependency, harness)) {
                    const bool is_own_dependency{ dependency.starts_with("headers") };

                    const fs::path corresponding_implementation_file{ (is_own_dependency
                        ? fs::path("src/" + dependency.substr(literal_length_of_headers)).replace_extension(is_c_file ? "c" : "cpp")
                        : fs::path([&dependency](){
                                const string symlink{ fs::read_symlink(dependency.substr(0, dependency.rfind("/"))).string() };

                                const size_t start{ symlink.rfind("dependencies/") };
                                const size_t stop{ symlink.rfind("/headers") };

                                const std::string shortened_symlink{ symlink.substr(start, stop - start) };

                                const size_t dep_start{ shortened_symlink.find("/") };
                                const size_t dep_stop{ shortened_symlink.find("@") };

                                const std::string dependency_name{ shortened_symlink.substr(dep_start + 1, dep_stop - dep_start - 1) };

                                return shortened_symlink + "/src/" + dependency.substr(dependency.find(dependency_name) + dependency_name.length() + 1);
                            }())
                        ).replace_extension(is_c_file ? "c" : "cpp")
                    };

                    if (fs::exists(corresponding_implementation_file)) {
                        const fs::path corresponding_binary{ (is_own_dependency
                            ? fs::path("build/binaries/" + dependency.substr(literal_length_of_headers))
                            : fs::path("build/dependencies/" + dependency.substr(literal_length_of_dependencies))
                            ).replace_extension("o")
                        };

                        if (!fs::exists(corresponding_binary)) {
                            throw std::runtime_error("Corresponding binary for '" + workspace::util::get_platform_formatted_filename(dependency) + "' not found! Run `cbt " + (is_own_dependency ? "compile-project" : "resolve-dependencies") + "`.");
                        } else {
                            files_to_link.push_back(corresponding_binary.string());
                        }
                    }
                }
            }

            const fs::path test_binary{ fs::path("build/test_binaries/unit_tests" / scoped_directory_of_file / fs::path(file).stem().replace_extension(EXTENSION)) };
            const int result = gnu_toolchain::create_test_binary(project, files_to_link, test_binary.string());
            
            cout << "[COMPILE]" << std::left << std::setw(6) << (result == 0 ? "[OK]" : "[NOK]") << workspace::util::get_platform_formatted_filename(test_binary) << endl;

            if (result == 0) {
                binaries_to_execute.push_back(test_binary);
            }
        }
        
        for (auto const& test_binary: binaries_to_execute) {
            [[maybe_unused]] const int result = gnu_toolchain::execute_test_binary(workspace::util::get_platform_formatted_filename(test_binary));
        }
    }

    void perform_static_analysis() {
        const Project project = convert_cfg_to_model();

        const string cmd{ string("cppcheck")
            + (" --std=" + project.config.cpp_standard)
            + " --language=c++"
            + " --enable=all"
            + " --quiet"
            + (project.cppcheck.bug_hunting.has_value() && project.cppcheck.bug_hunting.value() ? " --bug-hunting" : "")
            + (" --error-exitcode=" + std::to_string(project.cppcheck.error_exit_code))
            + (project.cppcheck.inconclusive.has_value() && project.cppcheck.inconclusive.value() ? " --inconclusive" : "")
            + (project.cppcheck.inline_suppression.has_value() && project.cppcheck.inline_suppression.value() ? " --inline-suppr" : "")
            + (" --platform=" + project.cppcheck.platform)
            + (project.cppcheck.safety.has_value() && project.cppcheck.safety.value() ? " --safety" : "")
            + (project.cppcheck.template_.has_value() ? (" --template=\"" + project.cppcheck.template_.value() + "\"") : "")
            + (project.cppcheck.verbose.has_value() && project.cppcheck.verbose.value() ? " --verbose" : "")
            + " -I headers/"
            + " src/"
        };

        cout << "[EXECUTE] " << cmd << "\n\n";

        [[maybe_unused]] const int result = system(cmd.c_str());
    }
    
    void show_info() {
        gnu_toolchain::CompilerInfo compiler = gnu_toolchain::get_compiler_info();

        cout
            << "cbt: C++ Build Tool" << endl
            << endl
            << "Author        - Swarnava Mukherjee" << endl
            << "Release       - 2026.02.06" << endl
            << endl
            << "C++ Standard  - " << __cplusplus << endl
            << "Compiler      - " << compiler.name << " (version "<< compiler.version << ")" << endl
            << "Platform      - " << compiler.platform << endl;
    }

    void show_help() {
        cout
            << "Usage:" << endl
            << endl
            << "1. Globally                     - cbt <options> [file_name]" << endl
            << "2. From inside a project        - [env=<an_env_file>] cbt <options> [file_name]" << endl
            << endl
            << "Note:" << endl
            << endl
            << "1. Environment files are located under the `<project>/environments/` directory" << endl
            << "2. The '.env.template' file must be provided with all the environment values along with the necessary types" << endl
            << "3. `env` entry defaults to 'local.env' if no `env` entry is provided" << endl
            << "4. Specify the `env` file to be picked up without the '.env' extension, e.g. \"env=production\"" << endl
            << endl
            << "Options:" << endl
            << endl
            << "create-application <name>       - Scaffold a new application" << endl
            << "create-library <name>           - Scaffold a new library" << endl
            << endl
            << "create-file <file_name>         - Generate respective C++ files under 'headers/', 'src/' and 'tests/unit_tests/' directories" << endl
            << "create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required" << endl
            << endl
            << "create-c-file <file_name>       - Generate respective C files under 'headers/c', 'src/c' and 'tests/unit_tests/c' directories (will create necessary sub-directories if required)" << endl
            << endl
            << "resolve-dependencies            - Sync dependencies through 'project.cfg'" << endl
            << endl
            << "compile-project                 - Compile all files and generate respective binaries under 'build/binaries/'" << endl
            << "build-project                   - (For applications only) Perform linking and generate final executable under 'build/'" << endl
            << "run-unit-tests                  - Run all test cases under 'tests/unit_tests/' directory" << endl
            << endl
            << "perform-static-analysis         - Invoke `cppcheck` with respective parameters from 'project.cfg'"  << endl
            << endl
            << "clear-build                     - Delete all object files under 'build/' directory"  << endl
            << endl
            << "info                            - Show information regarding cbt" << endl
            << "help                            - Shows this help message" << endl;
    }

    void show_usage() {
        cout << "Invalid usage. Type 'cbt help' for available commands." << endl;
    }
}
