#include "gnu_toolchain.hpp"

#include <numeric>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "workspace/project_config.hpp"

namespace {
    using namespace gnu_toolchain;

    enum class FoldType {
        BINARIES,
        PLAIN
    };
    
    std::string glob_directory_for_binaries(const std::string& directory) {
        return directory + SEPARATOR + "*.o";
    }

    auto fold_for_binaries = [](const std::string& acc, const std::string item) {
        return std::move(acc) + ' ' + glob_directory_for_binaries(item);
    };

    auto fold_plain = [](const std::string& acc, const std::string item) {
        return std::move(acc) + ' ' + item;
    };

    std::string join(const std::vector<std::string>& items, const FoldType& fold_type) {
        return fold_type == FoldType::BINARIES
            ? std::accumulate(
                std::next(items.begin()),
                items.end(),
                glob_directory_for_binaries(items[0]),
                fold_for_binaries)
            : std::accumulate(
                std::next(items.begin()),
                items.end(),
                items[0],
                fold_plain);
    }
}

namespace gnu_toolchain {
    using std::string;

    std::string get_compilation_command(const workspace::project_config::Project& project) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + INCLUDE_PATHS + " -c src/<FILE> -o build/binaries/<FILE>.o";
    }

    int compile_file(const workspace::project_config::Project& project, const string& input_file, const string& output_file) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.compile_time_flags + " " + INCLUDE_PATHS + " -c " + input_file + " -o build/binaries/" + output_file + ".o").c_str());
    }

    int perform_linking(const workspace::project_config::Project& project, const std::vector<string>& directories_containing_binaries, const string& executable_file, const bool echo) {
        const string command{ COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.build_flags + " " + join(directories_containing_binaries, FoldType::BINARIES) + " -o " + executable_file };

        if (echo) {
            std::cout << "[COMMAND] " << command << std::endl << std::endl;
        }

        return system(command.c_str());
    }

    string get_test_execution_command(const workspace::project_config::Project& project, const string& extension) {
        return COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + INCLUDE_PATHS + " tests/unit_tests/<FILE> -o build/test_binaries/unit_tests/<FILE>" + extension;
    }

    int create_test_binary(const workspace::project_config::Project& project, const std::vector<string>& files_to_link, const string& test_binary) {
        return system((COMPILER + " -std=" + project.config.cpp_standard + " " + project.config.safety_flags + " " + project.config.test_flags + " " + INCLUDE_PATHS + " " + join(files_to_link, FoldType::PLAIN) + " -o " + test_binary).c_str());
    }

    int execute_test_binary(const string& test_binary) {
        return system(test_binary.c_str());
    }
}