#include "gnu_toolchain.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "workspace/project_config.hpp"

namespace gnu_toolchain {
    using std::string;

    std::string get_compilation_command(const workspace::project_config::Project& project) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + INCLUDE_PATHS + " -c src/<FILE> -o build/binaries/<FILE>.o";
    }

    int compile_file(const workspace::project_config::Project& project, const string& input_file, const string& output_file) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + INCLUDE_PATHS + " -c " + input_file + " -o build/binaries/" + output_file + ".o").c_str());
    }

    int perform_linking(const workspace::project_config::Project& project, const string& binaries, const string& executable_file, const bool echo) {
        const string command{ COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.build_flags + " " + binaries + "-o " + executable_file };

        if (echo) {
            std::cout << "[COMMAND] " << command << std::endl << std::endl;
        }

        return system(command.c_str());
    }

    string get_test_execution_command(const workspace::project_config::Project& project, const string& extension) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + INCLUDE_PATHS + " tests/unit_tests/<FILE> -o build/test_binaries/unit_tests/<FILE>" + extension;
    }

    int create_test_binary(const workspace::project_config::Project& project, const string& files_to_link, const string& test_binary) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + INCLUDE_PATHS + " " + files_to_link + " -o " + test_binary).c_str());
    }

    int execute_test_binary(const string& test_binary) {
        return system(test_binary.c_str());
    }
}