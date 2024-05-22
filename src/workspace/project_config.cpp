#include "workspace/project_config.hpp"

#include <filesystem>
#include <string>

#include "workspace/scaffold.hpp"

namespace workspace::project_config {
    namespace fs = std::filesystem;

    using std::string;

    string platform_to_string(const Platform platform) {
        using enum workspace::project_config::Platform;
        
        switch(platform) {
            case BSD: return "bsd";
            case LINUX: return "linux";
            case MACOS: return "macos";
            case UNIX: return "unix";
            case WINDOWS: return "windows";
            default: return "<unsupported>";
        }
    }

    Platform string_to_platform(const string platform) {
        using enum workspace::project_config::Platform;

        if (platform.compare("bsd") == 0) { return BSD; }
        else if (platform.compare("linux") == 0) { return LINUX; }
        else if (platform.compare("macos") == 0) { return MACOS; }
        else if (platform.compare("unix") == 0) { return UNIX; }
        else if (platform.compare("windows") == 0) { return WINDOWS; }
        else return _UNSUPPORTED;
    }

    Project convert_cfg_to_model() {
        Project project;
        return project;
    }

    string convert_model_to_cfg(const Project project, const bool add_disclaimer_text) {
        const string disclaimer_text{ std::string("; Since a rudimentary INI parser is used, ensure that the actual `key` and `value` pairs")
            + "\n; follow the same `key` and `value` format in this file which was provided while creation"
            + "\n; of the project. Also, ensure that each pair is contained within a single line." };

        const string base_text{ std::string("name=") + project.name 
            + "\ndescription=" + project.description
            + "\nversion=" + project.version };
        
        string authors_text{ std::string("; `authors` is always an array even if there is only one entity. At least one author is required.") };

        for (const Author &author: project.authors) {
            authors_text += std::string("\nauthors[]=") + author.name + " <" + author.email_id + ">";
        }

        string platforms_text{ std::string("; `platforms` is always an array even if there is only one supported platform and")
            + "\n; values can be any of 'bsd', 'linux', 'macos', 'unix', `windows`. At least one platform is required." };
        
        for (const Platform &platform: project.platforms) {
            platforms_text += std::string("\nplatforms[]=") + platform_to_string(platform);
        }

        return (add_disclaimer_text ? (disclaimer_text + "\n\n") : "") + base_text + "\n\n" + authors_text + "\n\n" + platforms_text + "\n";
    }
}
