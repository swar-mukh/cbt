#ifndef WORKSPACE_SCAFFOLD
#define WORKSPACE_SCAFFOLD

#include <regex>
#include <string>

namespace workspace::scaffold {
    const std::regex IMPORT_R{ "@FILE_NAME" };
    const std::regex GUARD_R{ "@GUARD" };
    const std::regex NAMESPACE_R{ "@NAMESPACE" };
    const std::regex RELATIVE_SRC_R{ "@RELATIVE_SRC_FILE_NAME" };

    void create_file(const std::string project_name, const std::string file_name);
    bool create_directory(const std::string project_name, const std::string sub_directory = "", bool multi_directory = false, bool verbose = true);
    void create_build_tree_as_necessary();

    void exit_if_command_not_invoked_from_within_workspace();
}

#endif