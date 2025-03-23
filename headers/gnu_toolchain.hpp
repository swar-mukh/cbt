#ifndef GNU_TOOLCHAIN
#define GNU_TOOLCHAIN

#include "workspace/project_config.hpp"

#include <string>

namespace gnu_toolchain {
    using std::string;

    const string COMPILER{ "g++" };

    string get_compilation_command(const workspace::project_config::Project& project, const string& gpp_include_paths);
    int compile_file(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& input_file, const string& output_file);

    int perform_linking(const workspace::project_config::Project& project, const string& binaries, const string& executable_file, const bool echo = true);

    string get_test_execution_command(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& extension);
    int create_test_executable(const workspace::project_config::Project& project, const string& gpp_include_paths, const string& files_to_link, const string& test_executable_file);
    int execute_test_binary(const string& test_executable_file);
}

#endif
