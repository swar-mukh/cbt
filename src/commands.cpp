#include "commands.hpp"

#include <experimental/filesystem>
#include <iostream>

#include "util/scaffold.hpp"

namespace commands {
    namespace fs = std::experimental::filesystem;

    using std::cout;
    using std::endl;
    using std::string;
    
    void create_project(const string project_name) {
        if (fs::exists(project_name)) {
            cout << endl << "Directory '" << project_name << "' already exists!" << endl << endl;
            return;
        }

        cout << endl;

        if (util::scaffold::create_directory(project_name)) {
            util::scaffold::create_file(project_name, ".gitignore");
            util::scaffold::create_directory(project_name, ".project");
            util::scaffold::create_file(project_name, "README.md");
            util::scaffold::create_directory(project_name, "build");
            util::scaffold::create_directory(project_name, "build/binaries");
            util::scaffold::create_directory(project_name, "docs");
            util::scaffold::create_directory(project_name, "headers");
            util::scaffold::create_file(project_name, "headers/sample.hpp");
            util::scaffold::create_directory(project_name, "src");
            util::scaffold::create_file(project_name, "src/main.cpp");
            util::scaffold::create_file(project_name, "src/sample.cpp");

            cout << endl << "Project '" << project_name << "' created" << endl << endl;
        } else {
            cout << "Could not create project '" << project_name << "'!" << endl << endl;
        }
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
            << "Version       - 2023.11.06" << endl
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
            << "info                            - Show information regarding cbt" << endl
            << "help                            - Shows this help message" << endl
            << endl;
    }

    void show_usage() {
        cout << endl << "Invalid usage. Type 'cbt help' for available commands." << endl << endl;
    }
}
