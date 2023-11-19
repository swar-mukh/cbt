#include <iostream>
#include <string>
#include <vector>

#include "commands.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::string;
using std::vector;

void parse_commands_and_execute(vector <string> arguments) {
    try {
        if (arguments.size() == 3) {
            if (arguments[1].compare("create-project") == 0) {
                commands::create_project(arguments[2]);
            } else if (arguments[1].compare("create-file") == 0) {
                commands::create_file(arguments[2]);
            } else {
                commands::show_usage();
            }
        } else if (arguments.size() == 2) {
            if (arguments[1].compare("compile-project") == 0) {
                commands::compile_project();
            } else if (arguments[1].compare("clear-build") == 0) {
                commands::clear_build();
            } else if (arguments[1].compare("build-project") == 0) {
                commands::build_project();
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
    } catch (const exception & e) {
        cerr << "Exception: " << e.what() << endl;
    } catch (...) {
        cerr << "Something went wrong!";
    }
}

int main(const int argc, char *argv[]) {
    vector<string> args(argv, argv + argc);

    if (args.size() == 1) {
        commands::show_usage();
    } else {
        parse_commands_and_execute(args);
    }

    return EXIT_SUCCESS;
}
