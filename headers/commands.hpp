#ifndef COMMANDS
#define COMMANDS

#include <iostream>

namespace commands {
    void create_project(const std::string project_name);

    void clear_build();

    void show_info();
    void show_help();

    void show_usage();
}

#endif