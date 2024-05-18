#include "commands.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"

namespace commands {
    namespace fs = std::filesystem;

    using std::cout;
    using std::endl;
    using std::string;
    
    void create_project(const string project_name) {
        if (fs::exists(project_name)) {
            cout << "Directory '" << project_name << "' already exists!" << endl;
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
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << "Could not execute command! Are you sure you are inside the project workspace?" << endl;
            return;
        }
        
        const auto [is_valid, reason_if_any] = workspace::util::is_valid_file_name(file_name);

        if (!is_valid) {
            cout << reason_if_any << endl;
            return;
        }

        const bool create_only_header_file{ file_name.starts_with("headers/") };

        if (create_only_header_file) {
            workspace::scaffold::create_file(".", file_name + ".hpp");
        } else {
            workspace::scaffold::create_file(".", string("headers/") + file_name + ".hpp");
            workspace::scaffold::create_file(".", string("src/") + file_name + ".cpp");
            workspace::scaffold::create_file(".", string("tests/unit_tests/") + file_name + ".cpp");
        }
    }

    void compile_project() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << "Could not execute command! Are you sure you are inside the project workspace?" << endl;
            return;
        }

        bool show_newline_separator{false};

        if (!fs::exists("build/")) {
            show_newline_separator = true;

            workspace::scaffold::create_directory(string("."), string("build/binaries/"), true);
        }

        string gpp_include_paths{"-Iheaders"};
        const int literal_length_of_headers = string("headers/").length();
        const int literal_length_of_src = string("src/").length();
        const int literal_length_of_extension = string(".cpp").length();

        for (auto const& dir_entry: fs::recursive_directory_iterator("headers")) {
            if (fs::is_directory(dir_entry)) {
                const string directory = dir_entry.path().string();

                gpp_include_paths += " -I" + directory;

                const string directory_under_check = string("build/binaries/" + directory.substr(literal_length_of_headers));

                if (!fs::exists(directory_under_check)) {
                    workspace::scaffold::create_directory(string("."), directory_under_check, false, false);
                }
            }
        }

        if (show_newline_separator) {
            cout << endl;  
        }

        for (auto const& dir_entry: fs::recursive_directory_iterator("src")) {
            if (fs::is_regular_file(dir_entry)) {
                const string cpp_file = dir_entry.path().string();
                const string stemmed_cpp_file = cpp_file.substr(literal_length_of_src, cpp_file.length() - (literal_length_of_src + literal_length_of_extension));

                if (stemmed_cpp_file.compare("main") != 0 && !fs::exists("headers/" + stemmed_cpp_file + ".hpp")) {
                    cout << "SKIP " << ("headers/" + stemmed_cpp_file + ".hpp") << " (No corresponding file found!)" << endl;
                } else {
                    const int result = system((string("g++ -std=c++2a -Wall -Wextra -pedantic -Os ") + gpp_include_paths + " -c " + cpp_file + " -o build/binaries/" + stemmed_cpp_file + ".o").c_str());

                    cout << (result == 0 ? string("✔") : string("✘")) << " COMPILE " << cpp_file << " -> build/binaries/" << stemmed_cpp_file << ".o" <<  endl;
                }
            }
        }
    }

    void clear_build() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << "Could not execute command! Are you sure you are inside the project workspace?" << endl;
            return;
        }

        if (fs::remove_all(fs::current_path() / "build")) {
            cout << std::right << std::setw(8) << "DELETE " << "build/" << endl;
        }

        cout << endl;

        workspace::scaffold::create_directory(".", "build");
        workspace::scaffold::create_directory(".", "build/binaries");
        workspace::scaffold::create_directory(".", "build/test_binaries");
        workspace::scaffold::create_directory(".", "build/test_binaries/unit_tests");
    }

    void build_project() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << "Could not execute command! Are you sure you are inside the project workspace?" << endl;
            return;
        }

        if (!fs::exists("build/")) {
            cout << "Directory 'build/' does not exist!" << endl;
            return;
        }
        
        if (!fs::exists("build/binaries")) {
            cout << "Directory 'build/binaries/' does not exist!" << endl;
            return;
        }

        string binaries{ "" };
        int binary_files_count{ 0 };

        for (auto const& dir_entry: fs::recursive_directory_iterator("build/binaries")) {
            if (fs::is_regular_file(dir_entry)) {
                binary_files_count++;
                binaries += dir_entry.path().string() + " ";
            }
        }

        if (binary_files_count == 0) {
            cout << "No binaries present! Run 'cbt compile-project' first." << endl;
            return;
        }

        #if defined(_WIN32) || defined(_WIN64)
        const string BINARY_NAME = "app.exe";
        #else
        const string BINARY_NAME = "app";
        #endif

        const int result = system((string("g++ -std=c++2a -Wall -Wextra -pedantic -O3 -Os -s ") + binaries + "-o build/" + BINARY_NAME).c_str());

        cout << (result == 0 ? string("✔") : string("✘")) << (" BUILD build/" + BINARY_NAME) << endl;
    }

    void run_unit_tests() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << "Could not execute command! Are you sure you are inside the project workspace?" << endl;
            return;
        }

        bool show_newline_separator{false};

        if (!fs::exists("build/")) {
            show_newline_separator = true;

            workspace::scaffold::create_directory(string("."), string("build/test_binaries/unit_tests"), true);
        }

        if (show_newline_separator) {
            cout << endl;  
        }
         
        const string gpp_include_paths{ "-Iheaders" };
        const int literal_length_of_unit_tests = string("tests/unit_tests/").length();
        const int literal_length_of_extension = string(".cpp").length();

        #if defined(_WIN32) || defined(_WIN64)
        const string EXTENSION{ ".exe" };
        #else
        const string EXTENSION{ "" };
        #endif

        for (auto const& dir_entry: fs::recursive_directory_iterator("tests/unit_tests")) {
            if (fs::is_directory(dir_entry)) {
                const string directory = dir_entry.path().string();

                const string directory_under_check = string("build/test_binaries/unit_tests/" + directory.substr(literal_length_of_unit_tests));

                if (!fs::exists(directory_under_check)) {
                    workspace::scaffold::create_directory(string("."), directory_under_check, false, false);
                }
            } else if (fs::is_regular_file(dir_entry)) {
                const string cpp_file = dir_entry.path().string();
                const string stemmed_cpp_file = cpp_file.substr(literal_length_of_unit_tests, cpp_file.length() - (literal_length_of_unit_tests + literal_length_of_extension));

                const int result = system((string("g++ -std=c++2a -Wall -Wextra -pedantic -O3 -Os -s ") + gpp_include_paths + " " + cpp_file + " -o build/test_binaries/unit_tests/" + stemmed_cpp_file + EXTENSION).c_str());
                cout << (result == 0 ? string("✔") : string("✘")) << " COMPILE " << cpp_file << " -> build/test_binaries/unit_tests/" << (stemmed_cpp_file + EXTENSION) <<  endl;
            }
        }

        for (auto const& dir_entry: fs::recursive_directory_iterator("build/test_binaries/unit_tests/")) {
            if (fs::is_regular_file(dir_entry)) {
                fs::path cpp_path = dir_entry.path();

                #if defined(_WIN32) || defined(_WIN64)
                system((cpp_path.make_preferred().string()).c_str());
                #else
                system((cpp_path.string()).c_str());
                #endif
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
            << "Release       - 2024.05.15" << endl
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
            << "create-file <file_name>         - Generate header and respective C++ files under 'headers/', 'src/' and 'test/' directories" << endl
            << "create-file <path/to/file_name> - Same as above, but will create necessary sub-directories if required" << endl
            << endl
            << "compile-project                 - Compile all files and generate respective binaries under 'build/binaries/'" << endl
            << "clear-build                     - Delete all object files under 'build/' directory"  << endl
            << endl
            << "build-project                   - Perform linking and generate final executable under 'build/' (requires project compilation first)" << endl
            << endl
            << "run-unit-tests                  - Run all test cases under 'tests/unit_tests/' directory" << endl
            << endl
            << "info                            - Show information regarding cbt" << endl
            << "help                            - Shows this help message" << endl;
    }

    void show_usage() {
        cout << "Invalid usage. Type 'cbt help' for available commands." << endl;
    }
}
