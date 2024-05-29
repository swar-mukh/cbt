#include "commands.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "workspace/project_config.hpp"
#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"

namespace commands {
    namespace fs = std::filesystem;

    using std::cout;
    using std::endl;
    using std::string;
    using namespace workspace::project_config;
    
    void create_project(const string project_name) {
        if (fs::exists(project_name)) {
            cout << "Directory '" << project_name << "' already exists!" << endl;
            return;
        }

        const auto [is_valid, reason_if_any] = workspace::util::is_valid_project_name(project_name);

        if (!is_valid) {
            cout << reason_if_any << endl;
            return;
        }

        if (workspace::scaffold::create_directory(project_name)) {
            workspace::scaffold::create_file(project_name, ".gitignore");
            workspace::scaffold::create_directory(project_name, ".project");
            workspace::scaffold::create_directory(project_name, "build");
            workspace::scaffold::create_directory(project_name, "build/binaries");
            workspace::scaffold::create_directory(project_name, "build/test_binaries");
            workspace::scaffold::create_directory(project_name, "build/test_binaries/unit_tests");
            workspace::scaffold::create_directory(project_name, "docs");
            workspace::scaffold::create_file(project_name, "docs/LICENSE.txt");
            workspace::scaffold::create_file(project_name, "docs/Roadmap.md");
            workspace::scaffold::create_directory(project_name, "environments");
            workspace::scaffold::create_file(project_name, "environments/.env.template");
            workspace::scaffold::create_file(project_name, "environments/local.env");
            workspace::scaffold::create_file(project_name, "environments/production.env");
            workspace::scaffold::create_file(project_name, "environments/test.env");
            workspace::scaffold::create_directory(project_name, "headers");
            workspace::scaffold::create_directory(project_name, "headers/cbt_tools");
            workspace::scaffold::create_file(project_name, "headers/cbt_tools/env_manager.hpp");
            workspace::scaffold::create_file(project_name, "headers/cbt_tools/test_harness.hpp");
            workspace::scaffold::create_file(project_name, "headers/cbt_tools/utils.hpp");
            workspace::scaffold::create_file(project_name, "headers/sample.hpp");
            workspace::scaffold::create_directory(project_name, "src");
            workspace::scaffold::create_directory(project_name, "src/cbt_tools");
            workspace::scaffold::create_file(project_name, "src/cbt_tools/env_manager.cpp");
            workspace::scaffold::create_file(project_name, "src/cbt_tools/utils.cpp");
            workspace::scaffold::create_file(project_name, "src/main.cpp");
            workspace::scaffold::create_file(project_name, "src/sample.cpp");
            workspace::scaffold::create_directory(project_name, "tests");
            workspace::scaffold::create_directory(project_name, "tests/unit_tests");
            workspace::scaffold::create_file(project_name, "tests/unit_tests/sample.cpp");
            workspace::scaffold::create_file(project_name, "README.md");
            workspace::scaffold::create_file(project_name, "project.cfg");

            cout << endl << "Project '" << project_name << "' created" << endl;
        } else {
            cout << "Could not create project '" << project_name << "'!" << endl;
        }
    }

    void create_file(const string file_name) {
        const auto [is_valid, reason_if_any] = workspace::util::is_valid_file_name(file_name);

        if (!is_valid) {
            cout << reason_if_any << endl;
            return;
        }

        const bool create_only_header_file{ file_name.starts_with("headers/") };

        if (create_only_header_file) {
            workspace::scaffold::create_file("", file_name + ".hpp");
        } else {
            workspace::scaffold::create_file("", string("headers/") + file_name + ".hpp");
            workspace::scaffold::create_file("", string("src/") + file_name + ".cpp");
            workspace::scaffold::create_file("", string("tests/unit_tests/") + file_name + ".cpp");
        }
    }

    void compile_project() {
        workspace::scaffold::create_build_tree_as_necessary();

        const string gpp_include_paths{"-Iheaders"};
        const int literal_length_of_headers = string("headers/").length();
        const int literal_length_of_src = string("src/").length();
        const int literal_length_of_extension = string(".cpp").length();

        const Project project = convert_cfg_to_model();

        for (auto const& dir_entry: fs::recursive_directory_iterator("headers")) {
            if (fs::is_directory(dir_entry)) {
                const string directory = dir_entry.path().string();
                const string directory_under_check = string("build/binaries/" + directory.substr(literal_length_of_headers));

                if (!fs::exists(directory_under_check)) {
                    workspace::scaffold::create_directory(string("."), directory_under_check, false, false);
                }
            }
        }

        cout << "[COMMAND] " << ("g++ -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + gpp_include_paths + " -c src/<FILE> -o build/binaries/<FILE>.o") << endl << endl;

        for (auto const& dir_entry: fs::recursive_directory_iterator("src")) {
            if (fs::is_regular_file(dir_entry)) {
                const string cpp_file = dir_entry.path().string();
                const string stemmed_cpp_file = cpp_file.substr(literal_length_of_src, cpp_file.length() - (literal_length_of_src + literal_length_of_extension));

                if (stemmed_cpp_file.compare("main") != 0 && !fs::exists("headers/" + stemmed_cpp_file + ".hpp")) {
                    cout << "SKIP " << ("headers/" + stemmed_cpp_file + ".hpp") << " (No corresponding file found!)" << endl;
                } else {
                    const int result = system((string("g++") + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + gpp_include_paths + " -c " + cpp_file + " -o build/binaries/" + stemmed_cpp_file + ".o").c_str());

                    cout << "[COMPILE]" << std::left << std::setw(6) << (result == 0 ? "[OK]" : "[NOK]") << cpp_file <<  endl;
                }
            }
        }
    }

    void clear_build() {
        if (fs::remove_all(fs::current_path() / "build")) {
            cout << std::right << std::setw(8) << "RECREATE " << "build/" << endl;
        }

        workspace::scaffold::create_build_tree_as_necessary();
    }

    void build_project() {
        if (!fs::exists("build/")) {
            cout << "Directory 'build/' does not exist! Run 'cbt compile-project' first." << endl;
            return;
        }
        
        if (!fs::exists("build/binaries")) {
            cout << "Directory 'build/binaries/' does not exist! Run 'cbt compile-project' first." << endl;
            return;
        }

        string binaries{ "build/binaries/*.o " };
        int binary_files_count{ 0 };

        for (auto const& dir_entry: fs::recursive_directory_iterator("build/binaries")) {
            if (fs::is_directory(dir_entry)) {
                binaries += dir_entry.path().string() + "/*.o ";
            } else if (fs::is_regular_file(dir_entry)) {
                binary_files_count++;
            }
        }

        if (binary_files_count == 0) {
            cout << "No binaries present! Run 'cbt compile-project' first." << endl;
            return;
        }

        const Project project = convert_cfg_to_model();

        #if defined(_WIN32) || defined(_WIN64)
        const string BINARY_NAME{ project.name + ".exe" };
        #else
        const string BINARY_NAME{ project.name };
        #endif

        cout << "[COMMAND] " << ("g++ -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.build_flags + " " + binaries + "-o build/" + BINARY_NAME) << endl << endl;

        const int result = system((string("g++") + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.build_flags + " " + binaries + "-o build/" + BINARY_NAME).c_str());

        cout << "[BUILD]" << std::left << std::setw(6) << (result == 0 ? "[OK]" : "[NOK]") << workspace::util::get_platform_formatted_filename("build/" + BINARY_NAME) << endl;
    }

    void run_unit_tests() {
        workspace::scaffold::create_build_tree_as_necessary();
         
        const string gpp_include_paths{ "-Iheaders" };
        const string unit_tests_directory{ "tests/unit_tests/" };
        const int literal_length_of_unit_tests = unit_tests_directory.length();
        const int literal_length_of_extension = string(".cpp").length();

        const Project project = convert_cfg_to_model();

        #if defined(_WIN32) || defined(_WIN64)
        const string EXTENSION{ ".exe" };
        #else
        const string EXTENSION{ "" };
        #endif

        cout << "[COMMAND] " << ("g++ -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + gpp_include_paths + " " + unit_tests_directory + "<FILE> -o build/test_binaries/unit_tests/<FILE>" + EXTENSION) << endl << endl;

        for (auto const& dir_entry: fs::recursive_directory_iterator(unit_tests_directory)) {
            if (fs::is_directory(dir_entry)) {
                const string directory = dir_entry.path().string();

                const string directory_under_check = string("build/test_binaries/unit_tests/" + directory.substr(literal_length_of_unit_tests));

                if (!fs::exists(directory_under_check)) {
                    workspace::scaffold::create_directory(string("."), directory_under_check, false, false);
                }
            } else if (fs::is_regular_file(dir_entry)) {
                const string cpp_file = dir_entry.path().string();
                const string stemmed_cpp_file = cpp_file.substr(literal_length_of_unit_tests, cpp_file.length() - (literal_length_of_unit_tests + literal_length_of_extension));

                const int result = system((string("g++") + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + gpp_include_paths + " " + cpp_file + " -o build/test_binaries/unit_tests/" + stemmed_cpp_file + EXTENSION).c_str());
                cout << "[COMPILE]" << std::left << std::setw(6) << (result == 0 ? "[OK]" : "[NOK]") << workspace::util::get_platform_formatted_filename(cpp_file) <<  endl;
            }
        }

        for (auto const& dir_entry: fs::recursive_directory_iterator("build/test_binaries/unit_tests/")) {
            if (fs::is_regular_file(dir_entry)) {
                [[maybe_unused]] const int result = system(workspace::util::get_platform_formatted_filename(dir_entry.path()).c_str());
            }
        }
    }

    void show_info() {
        const string GCC_VERSION = std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);

        #ifdef __APPLE__
        const string PLATFORM = "Apple MacOS";
        #elif __linux__
        const string PLATFORM = "Linux";
        #elif __unix__
        const string PLATFORM = "Unix";
        #elif defined(_WIN32) || defined(_WIN64)
        const string PLATFORM = "Microsoft Windows";
        #endif

        cout
            << "cbt: C++ Build Tool" << endl
            << endl
            << "Author        - Swarnava Mukherjee" << endl
            << "Release       - 2024.05.26" << endl
            << endl
            << "C++ Standard  - " << __cplusplus << endl
            << "GCC Version   - " << GCC_VERSION << endl
            << "Platform      - " << PLATFORM << endl;
    }

    void show_help() {
        cout
            << "Usage:" << endl
            << endl
            << "[env=<an_env_file>] cbt <options> [file_name]" << endl
            << endl
            << "Note:" << endl
            << endl
            << "1. Environment files are located under the `environments/` directory" << endl
            << "2. The '.env.template' file must be provided with all the environment values along with the necessary types" << endl
            << "3. `env` entry defaults to 'local.env' if no `env` entry is provided" << endl
            << "4. Specify the `env` file to be picked up without the '.env' extension, e.g. \"env=production\"" << endl
            << endl
            << "Options:" << endl
            << endl
            << "create-project <project-name>   - Scaffold a new project" << endl
            << endl
            << "create-file <file_name>         - Generate respective C++ files under 'headers/', 'src/' and 'tests/' directories" << endl
            << "create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required" << endl
            << endl
            << "compile-project                 - Compile all files and generate respective binaries under 'build/binaries/'" << endl
            << "build-project                   - Perform linking and generate final executable under 'build/' (requires project compilation first)" << endl
            << "run-unit-tests                  - Run all test cases under 'tests/unit_tests/' directory" << endl
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
