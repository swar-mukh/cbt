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
            cout << endl << "Directory '" << project_name << "' already exists!" << endl << endl;
            return;
        }

        cout << endl;

        if (workspace::scaffold::create_directory(project_name)) {
            workspace::scaffold::create_file(project_name, ".gitignore");
            workspace::scaffold::create_directory(project_name, ".project");
            workspace::scaffold::create_directory(project_name, "build");
            workspace::scaffold::create_directory(project_name, "build/binaries");
            workspace::scaffold::create_directory(project_name, "docs");
            workspace::scaffold::create_file(project_name, "docs/LICENSE.txt");
            workspace::scaffold::create_file(project_name, "docs/Roadmap.md");
            workspace::scaffold::create_directory(project_name, "headers");
            workspace::scaffold::create_file(project_name, "headers/sample.hpp");
            workspace::scaffold::create_directory(project_name, "src");
            workspace::scaffold::create_file(project_name, "src/main.cpp");
            workspace::scaffold::create_file(project_name, "src/sample.cpp");
            workspace::scaffold::create_file(project_name, "README.md");
            workspace::scaffold::create_file(project_name, "project.cfg");

            cout << endl << "Project '" << project_name << "' created" << endl << endl;
        } else {
            cout << "Could not create project '" << project_name << "'!" << endl << endl;
        }
    }

    void create_file(const string file_name) {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << endl << "Could not execute command! Are you sure you are inside the project workspace?" << endl << endl;
            return;
        }
        
        const auto [is_valid, reason_if_any] = workspace::util::is_valid_file_name(file_name);

        if (!is_valid) {
            cout << endl << reason_if_any << endl << endl;
            return;
        }

        cout << endl;

        const bool create_only_header_file{ file_name.starts_with("headers/") };

        if (create_only_header_file) {
            workspace::scaffold::create_file(".", file_name + ".hpp");
        } else {
            workspace::scaffold::create_file(".", string("headers/") + file_name + ".hpp");
            workspace::scaffold::create_file(".", string("src/") + file_name + ".cpp");
        }

        cout << endl;
    }

    void compile_project() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << endl << "Could not execute command! Are you sure you are inside the project workspace?" << endl << endl;
            return;
        }

        cout << endl;

        bool show_newline_separator{false};

        if (!fs::exists("build/")) {
            show_newline_separator = true;

            workspace::scaffold::create_directory(string("."), string("build"));
            workspace::scaffold::create_directory(string("."), string("build/binaries/"));
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
                    show_newline_separator = true;
                    workspace::scaffold::create_directory(string("."), directory_under_check);
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
                    const int result = system((string("g++ -std=c++17 -Wall -Wextra -pedantic ") + gpp_include_paths + " -c " + cpp_file + " -o build/binaries/" + stemmed_cpp_file + ".o").c_str());

                    cout << (result == 0 ? string("✔") : string("✘")) << " COMPILE " << cpp_file << " -> build/binaries/" << stemmed_cpp_file << ".o" <<  endl;
                }
            }
        }

        cout << endl;
    }

    void clear_build() {
        if (!workspace::scaffold::is_command_invoked_from_workspace()) {
            cout << endl << "Could not execute command! Are you sure you are inside the project workspace?" << endl << endl;
            return;
        }

        if (fs::remove_all(fs::current_path() / "build")) {
            cout << endl << "DELETE build/" << endl;
        }

        cout << endl;

        workspace::scaffold::create_directory(".", "build");
        workspace::scaffold::create_directory(".", "build/binaries");

        cout << endl;
    }

    void show_info() {
        const string GNU_VERSION = std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);

        #ifdef __APPLE__
        const string PLATFORM = "Apple MacOS";
        #elif __linux__
        const string PLATFORM = "Linux";
        #elif __unix__
        const string PLATFORM = "Unix";
        #elif defined(_WIN32) || defined(_WIN64)
        const string PLATFORM = "Microsoft Windows";
        #endif

        cout << endl
            << "cbt: C++ Build Tool" << endl
            << endl
            << "Author        - Swarnava Mukherjee" << endl
            << "Version       - 2023.11.11" << endl
            << endl
            << "C++ Standard  - " << __cplusplus << endl
            << "GNU Version   - " << GNU_VERSION << endl
            << "Platform      - " << PLATFORM << endl
            << endl;
    }

    void show_help() {
        cout << endl
            << "Usage:" << endl
            << endl
            << "cbt <options> [items]" << endl
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
            << "info                            - Show information regarding cbt" << endl
            << "help                            - Shows this help message" << endl
            << endl;
    }

    void show_usage() {
        cout << endl << "Invalid usage. Type 'cbt help' for available commands." << endl << endl;
    }
}
