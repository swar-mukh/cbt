#ifndef WORKSPACE_MODIFICATION_IDENTIFIER
#define WORKSPACE_MODIFICATION_IDENTIFIER

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "workspace/project_config.hpp"

namespace workspace::modification_identifier {
    namespace fs = std::filesystem;

    using FileHash = decltype(fs::hash_value(std::declval<fs::path>()));

    struct SourceFile {
        FileHash hash;
        std::string file_name;

        std::size_t last_modified_timestamp;
        mutable std::size_t compilation_start_timestamp;
        mutable std::size_t compilation_end_timestamp;

        mutable bool affected;
        mutable bool was_successful;

        bool operator<(const SourceFile& another_file) const {
            return this->hash < another_file.hash;
        }
    };

    using SourceFiles = std::set<SourceFile>;
    using RawDependencyTree = std::map<std::string, std::vector<std::string>>;

    std::size_t get_current_fileclock_timestamp();
    SourceFiles list_all_files_annotated(const workspace::project_config::Project& project, const bool compile_as_dependency = false);
    RawDependencyTree get_files_to_test(const workspace::project_config::Project& project);
    void persist_annotations(const SourceFiles& bucket);
}

#endif
