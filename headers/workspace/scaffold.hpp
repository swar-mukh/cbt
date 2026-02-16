#ifndef WORKSPACE_SCAFFOLD
#define WORKSPACE_SCAFFOLD

#include <optional>
#include <string>

#include "workspace/modification_identifier.hpp"
#include "workspace/project_config.hpp"

namespace workspace::scaffold {
    using std::string;

    void create_file(const std::optional<workspace::project_config::Project> project, const string& file_name, const bool verbose = true, const bool skip_root = false);
    bool create_directory(const string& project_name, const string& sub_directory = "", const bool multi_directory = false, const bool verbose = true);
    
    void create_working_tree_as_necessary();

    void purge_old_binaries(const string& path, const workspace::modification_identifier::SourceFiles& annotated_files);

    void remove_dependency(const string& dependency, const string& version);
    void make_dependency_pristine(const string& dependency);

    void exit_if_command_not_invoked_from_within_workspace();
}

#endif
