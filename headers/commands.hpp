#ifndef COMMANDS
#define COMMANDS

#include <string>

namespace commands {
    void create_project(const std::string project_name);

    void clear_build();

    void show_info();
    void show_help();

    void show_usage();
}

#endif