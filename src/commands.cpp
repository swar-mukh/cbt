#include "commands.hpp"

#include <experimental/filesystem>
#include <fstream>
#include <iostream>

#include "util/text_content.hpp"

namespace commands {
    namespace fs = std::experimental::filesystem;

    using std::cout;
    using std::endl;
    using std::ofstream;
    using std::string;

    void __create_file(const string project_name, const string file_name) {
        ofstream file_to_write(project_name + file_name);
            
        file_to_write << util::text_content::get_predefined_text_content(file_name);
        
        file_to_write.close();

        cout << "CREATE " << project_name + file_name << endl;
    }
    
    void create_project(const string project_name) {
        if (fs::exists(project_name)) {
            cout << endl << "Directory '" << project_name << "' already exists!" << endl << endl;
            return;
        }

        if (fs::create_directory(project_name)) {
            __create_file(project_name, "/.gitignore");
            fs::create_directory(project_name + "/.project") ? cout << "CREATE " << project_name + "/.project" << endl : cout;
            __create_file(project_name, "/README.md");
            fs::create_directory(project_name + "/build") ? cout << "CREATE " << project_name + "/build" << endl : cout;
            fs::create_directory(project_name + "/build/binaries") ? cout << "CREATE " << project_name + "/build/binaries" << endl : cout;
            fs::create_directory(project_name + "/docs") ? cout << "CREATE " << project_name + "/docs" << endl : cout;
            fs::create_directory(project_name + "/headers") ? cout << "CREATE " << project_name + "/headers" << endl : cout;
            __create_file(project_name, "/headers/sample.hpp");
            fs::create_directory(project_name + "/src") ? cout << "CREATE " << project_name + "/src" << endl : cout;
            __create_file(project_name, "/src/main.cpp");
            __create_file(project_name, "/src/sample.cpp");

            cout << endl << "Project '" << project_name << "' created" << endl << endl;
        } else {
            cout << endl << "Could not create project '" << project_name << "'!" << endl << endl;
        }
    }

    void show_help() {
        cout << endl
            << "Usage:" << endl
            << endl
            << "cbt <options> [items]" << endl
            << endl
            << "Options:" << endl
            << endl
            << "create-project <project-name> - Scaffold a new project" << endl
            << "help                          - Shows this help message" << endl
            << endl;
    }

    void show_usage() {
        cout << endl << "Invalid usage. Type 'cbt help' for available commands." << endl << endl;
    }
}
