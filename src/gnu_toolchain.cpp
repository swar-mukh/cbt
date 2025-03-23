#include "gnu_toolchain.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "workspace/project_config.hpp"

namespace gnu_toolchain {
    using std::string;

    std::string get_compilation_command(const workspace::project_config::Project& project, const string& gpp_include_paths) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + gpp_include_paths + " -c src/<FILE> -o build/binaries/<FILE>.o";
    }

    int compile_file(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& input_file, const string& output_file) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + gpp_include_paths + " -c " + input_file + " -o build/binaries/" + output_file + ".o").c_str());
    }

    int perform_linking(const workspace::project_config::Project& project, const string& binaries, const string& executable_file, const bool echo) {
        const string command{ COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.build_flags + " " + binaries + "-o " + executable_file };

        if (echo) {
            std::cout << "[COMMAND] " << command << std::endl << std::endl;
        }

        return system(command.c_str());
    }

    string get_test_execution_command(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& extension) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + gpp_include_paths + " tests/unit_tests/<FILE> -o build/test_binaries/unit_tests/<FILE>" + extension;
    }

    int create_test_executable(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& files_to_link, const string& test_executable_file) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + gpp_include_paths + " " + files_to_link + " -o " + test_executable_file).c_str());
    }

    int execute_test_binary(const string& test_executable_file) {
        return system(test_executable_file.c_str());
    }
}