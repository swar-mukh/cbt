#ifndef WORKSPACE_SCAFFOLD
#define WORKSPACE_SCAFFOLD

#include <regex>
#include <string>

namespace workspace::scaffold {
    const std::regex IMPORT_R{ "@FILE_NAME" };
    const std::regex GUARD_R{ "@GUARD" };
    const std::regex NAMESPACE_R{ "@NAMESPACE" };

    void create_file(const std::string project_name, const std::string file_name);
    bool create_directory(const std::string project_name, const std::string sub_directory = "", bool multi_directory = false);
    std::string get_predefined_text_content(const std::string file_name);
    bool is_command_invoked_from_workspace();
}

#endif