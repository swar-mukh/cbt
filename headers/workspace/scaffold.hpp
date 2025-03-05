#ifndef WORKSPACE_SCAFFOLD
#define WORKSPACE_SCAFFOLD

#include <regex>
#include <string>

#include "workspace/modification_identifier.hpp"
#include "workspace/project_config.hpp"

namespace workspace::scaffold {
    using std::string;

    const std::regex PROJECT_NAME_R{ "@PROJECT_NAME" };
    const std::regex IMPORT_R{ "@FILE_NAME" };
    const std::regex GUARD_R{ "@GUARD" };
    const std::regex NAMESPACE_R{ "@NAMESPACE" };
    const std::regex RELATIVE_SRC_R{ "@RELATIVE_SRC_FILE_NAME" };
    const std::regex START_SCOPE_R{ "@START_SCOPE" };
    const std::regex END_SCOPE_R{ "@END_SCOPE" };

    void create_file(const string& project_name, const string& file_name, const workspace::project_config::ProjectType& project_type = workspace::project_config::ProjectType::APPLICATION, const bool verbose = true);
    bool create_directory(const string& project_name, const string& sub_directory = "", const bool multi_directory = false, const bool verbose = true);
    void create_build_tree_as_necessary();
    void create_internals_tree_as_necessary();

    void purge_old_binaries(const string& path, const workspace::modification_identifier::SourceFiles& annotated_files);

    void exit_if_command_not_invoked_from_within_workspace();
}

#endif
