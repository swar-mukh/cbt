#ifndef WORKSPACE_SCAFFOLD
#define WORKSPACE_SCAFFOLD

#include <string>

namespace workspace::scaffold {
    void create_file(const std::string project_name, const std::string file_name);
    bool create_directory(const std::string project_name, const std::string sub_directory = "");
    std::string get_predefined_text_content(const std::string file_name);
    bool is_command_invoked_from_workspace();
}

#endif