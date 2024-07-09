#ifndef WORKSPACE_MODIFICATION_IDENTIFIER
#define WORKSPACE_MODIFICATION_IDENTIFIER

#include <filesystem>
#include <set>
#include <string>

#include "workspace/project_config.hpp"

namespace workspace::modification_identifier {
    namespace fs = std::filesystem;

    struct SourceFile {
        std::size_t hash;
        std::string file_name;

        std::size_t last_modified_timestamp;
        mutable std::size_t compilation_start_timestamp;
        mutable std::size_t compilation_end_timestamp;

        mutable bool affected;
        mutable bool was_successful;
    };

    struct __SourceFileComparator {
        bool operator()(const SourceFile& left, const SourceFile& right) const {
            return left.hash < right.hash;
        }
    };

    using SourceFiles = std::set<SourceFile, __SourceFileComparator>;

    std::size_t get_current_fileclock_timestamp();
    SourceFiles list_all_files_annotated(const workspace::project_config::Project& project);
    void persist_annotations(const SourceFiles& bucket);
}

#endif
