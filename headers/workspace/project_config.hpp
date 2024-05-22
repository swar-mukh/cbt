#ifndef WORKSPACE_PROJECT_CONFIG
#define WORKSPACE_PROJECT_CONFIG

#include <list>
#include <string>

namespace workspace::project_config {
    using std::string;

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
    };

    struct Project {
        string name;
        string description;
        string version;

        std::list<Author> authors;
        std::list<Platform> platforms;
    };

    string platform_to_string(const Platform platform);
    Platform string_to_platform(const string platform);

    Project convert_cfg_to_model();
    string convert_model_to_cfg(const Project project, const bool add_disclaimer_text = true);
}

#endif
