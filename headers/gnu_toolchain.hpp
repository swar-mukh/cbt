#ifndef GNU_TOOLCHAIN
#define GNU_TOOLCHAIN

#include "workspace/project_config.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace gnu_toolchain {
    using std::string;

    const string COMPILER{ "g++" };
    const string INCLUDE_PATHS{ "-Iheaders -Idependencies" };
    const string INCLUDE_PATHS_FOR_DEPENDENCIES{ "-Iheaders -I../../dependencies" };

    const std::string SEPARATOR{ std::filesystem::path::preferred_separator };

    int generate_makefile(const workspace::project_config::Project& project, const string& files, const bool compile_as_dependency = false);

    string get_compilation_command(const workspace::project_config::Project& project, const bool compile_as_dependency = false);
    int compile_file(const workspace::project_config::Project& project, const string& input_file, const string& output_file, const bool compile_as_dependency = false);

    int perform_linking(const workspace::project_config::Project& project, const std::vector<string>& directories_containing_binaries, const string& executable_file, const bool echo = true);

    string get_test_execution_command(const workspace::project_config::Project& project, const string& extension);
    int create_test_binary(const workspace::project_config::Project& project, const std::vector<string>& files_to_link, const string& test_binary);
    int execute_test_binary(const string& test_binary);
}

#endif
