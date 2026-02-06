#ifndef WORKSPACE_PROJECT_CONFIG
#define WORKSPACE_PROJECT_CONFIG

#include <set>
#include <string>

namespace workspace::project_config {
    using std::string;
    
    enum class ProjectType {
        APPLICATION,
        LIBRARY
    };

    enum class Platform {
        BSD,
        LINUX,
        MACOS,
        UNIX,
        WINDOWS,
        _UNSUPPORTED
    };

    struct Author {
        string name;
        string email_id;
        
        bool operator<(const Author &another_author) const {
            return this->name < another_author.name;
        }
    };

    struct Config {
        string cpp_standard;
        string safety_flags;
        string compile_time_flags;
        string build_flags;
        string test_flags;
    };

    struct SurfaceDependency {
        string name;
        string version;
        string url;
    };

    struct SurfaceDependencyComparator {
        using is_transparent = void;

        bool operator()(const SurfaceDependency& lhs, const SurfaceDependency& rhs) const {
            if (lhs.name == rhs.name) {
                return lhs.version > rhs.version;
            }

            return lhs.name < rhs.name;
        }

        bool operator()(const SurfaceDependency& lhs, const std::string& rhs) const {
            return lhs.name < rhs;
        }

        bool operator()(const std::string& lhs, const SurfaceDependency& rhs) const {
            return lhs < rhs.name;
        }
    };

    using SurfaceDependencies = std::set<SurfaceDependency, SurfaceDependencyComparator>;

    struct Project {
        string name;
        string description;
        string version;

        ProjectType project_type;

        std::set<Author> authors;
        std::set<Platform> platforms;
        Config config;

        SurfaceDependencies dependencies;
    };

    struct ProjectComparator {
        using is_transparent = void;

        bool operator()(const Project& lhs, const Project& rhs) const {
            if (lhs.name == rhs.name) {
                return lhs.version > rhs.version;
            }

            return lhs.name < rhs.name;
        }

        bool operator()(const Project& lhs, const SurfaceDependency& rhs) const {
            if (lhs.name == rhs.name) {
                return lhs.version > rhs.version;
            }

            return lhs.name < rhs.name;
        }

        bool operator()(const SurfaceDependency& lhs, const Project& rhs) const {
            if (lhs.name == rhs.name) {
                return lhs.version > rhs.version;
            }

            return lhs.name < rhs.name;
        }
    };

    using Projects = std::set<Project, ProjectComparator>;

    Project init(const string& name, const ProjectType& project_type);

    string platform_to_string(const Platform& platform);
    Platform string_to_platform(const string& platform);

    string project_type_to_string(const ProjectType& project_type);
    ProjectType string_to_project_type(const string& project_type);

    std::string dependency_to_string(const SurfaceDependency& dependency, const bool exclude_url = true);
    SurfaceDependency parse_dependency(const string& value);

    Project convert_cfg_to_model();
    string convert_model_to_cfg(const Project& project, const bool add_disclaimer_text = true);
}

#endif
