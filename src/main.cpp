#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "commands.hpp"
#include "workspace/scaffold.hpp"

void parse_commands_and_execute(std::vector<std::string>& arguments) {
    try {
        if (arguments.size() == 3) {
            if (arguments[1].compare("create-application") == 0) {
                commands::create_application(arguments[2]);
            } else if (arguments[1].compare("create-library") == 0) {
                commands::create_library(arguments[2]);
            } else if (arguments[1].compare("create-file") == 0) {
                workspace::scaffold::exit_if_command_not_invoked_from_within_workspace();
                commands::create_file(arguments[2]);
            } else {
                commands::show_usage();
            }
        } else if (arguments.size() == 2) {
            if (arguments[1].compare("compile-project") == 0) {
                workspace::scaffold::exit_if_command_not_invoked_from_within_workspace();
                commands::compile_project();
            } else if (arguments[1].compare("clear-build") == 0) {
                workspace::scaffold::exit_if_command_not_invoked_from_within_workspace();
                commands::clear_build();
            } else if (arguments[1].compare("build-project") == 0) {
                workspace::scaffold::exit_if_command_not_invoked_from_within_workspace();
                commands::build_project();
            } else if (arguments[1].compare("run-unit-tests") == 0) {
                workspace::scaffold::exit_if_command_not_invoked_from_within_workspace();
                commands::run_unit_tests();
            } else if (arguments[1].compare("info") == 0) {
                commands::show_info();
            } else if (arguments[1].compare("help") == 0) {
                commands::show_help();
            } else {
                commands::show_usage();
            }
        } else {
            commands::show_usage();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Something went wrong!" << std::endl << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main(const int argc, char *argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::cout << std::endl;

    if (args.size() == 1) {
        commands::show_usage();
    } else {
        parse_commands_and_execute(args);
    }

    std::cout << std::endl;

    return EXIT_SUCCESS;
}
