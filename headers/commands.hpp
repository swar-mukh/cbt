#ifndef COMMANDS
#define COMMANDS

#include <string>

namespace commands {
    void create_application(const std::string& project_name);
    void create_library(const std::string& project_name);
    
    void create_file(const std::string& file_name);

    void resolve_dependencies();

    void compile_project(const bool compile_as_dependency = false);
    void clear_build();

    void build_project();

    void run_unit_tests();
    
    void show_info();
    void show_help();

    void show_usage();
}

#endif
