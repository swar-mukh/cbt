#include "workspace/project_config.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"

namespace workspace::project_config {
    namespace fs = std::filesystem;

    using std::string;

    const string AUTHOR_DELIMITER{ ":" };
    const string DELIMITER{ "=" };
    const string LINE_COMMENT{ ";" };

    string platform_to_string(const Platform& platform) {
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

    Platform string_to_platform(const string& platform) {
        using enum workspace::project_config::Platform;

        if (platform.compare("bsd") == 0) { return BSD; }
        else if (platform.compare("linux") == 0) { return LINUX; }
        else if (platform.compare("macos") == 0) { return MACOS; }
        else if (platform.compare("unix") == 0) { return UNIX; }
        else if (platform.compare("windows") == 0) { return WINDOWS; }
        else return _UNSUPPORTED;
    }

    string project_type_to_string(const ProjectType& project_type) {
        using enum workspace::project_config::ProjectType;
        
        switch(project_type) {
            case APPLICATION: return "application";
            case LIBRARY: return "library";
            default: return "<unsupported>";
        }
    }

    ProjectType string_to_project_type(const string& project_type) {
        using enum workspace::project_config::ProjectType;

        if (project_type.compare("application") == 0) { return APPLICATION; }
        else if (project_type.compare("library") == 0) { return LIBRARY; }
        else {
            throw std::domain_error("Invalid project type");
        };
    }

    Project convert_cfg_to_model() {
        const string config_file_name{ "project.cfg" };

        if (fs::exists(config_file_name)) {
            std::ifstream config_file(config_file_name);
            string raw_line{ "" }, line{ "" };
            int line_number{ 0 };

            Project project;

            while (std::getline(config_file, raw_line)) {
                ++line_number;
                std::erase(raw_line, '\r');

                if (raw_line.empty() || raw_line.starts_with(LINE_COMMENT)) {
                    continue;
                }

                const auto index_of_comment = raw_line.find(';');
                
                line = index_of_comment != string::npos ?
                    raw_line.substr(0, index_of_comment)
                    : raw_line;
                
                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, DELIMITER);
            
                if (key.compare("name") == 0) {
                    const auto [is_valid, reason_if_any] = workspace::util::is_valid_project_name(value);

                    if (!is_valid) {
                        throw std::runtime_error(reason_if_any);
                    }

                    project.name = value;
                } else if (key.compare("description") == 0) {
                    project.description = value;
                } else if (key.compare("version") == 0) {
                    project.version = value;
                } else if (key.compare("type") == 0) {
                    project.project_type = string_to_project_type(value);
                } else if (key.compare("authors[]") == 0) {
                    const auto [name, email_id] = workspace::util::get_key_value_pair_from_line(value, AUTHOR_DELIMITER);
                    project.authors.insert(Author{ .name{ name }, .email_id{ email_id } });
                } else if (key.compare("platforms[]") == 0) {
                    project.platforms.insert(string_to_platform(value));
                } else if (key.compare("config{cpp_standard}") == 0) {
                    project.config.cpp_standard = value;
                } else if (key.compare("config{safety_flags}") == 0) {
                    project.config.safety_flags = value;
                } else if (key.compare("config{compile_time_flags}") == 0) {
                    project.config.compile_time_flags = value;
                } else if (key.compare("config{build_flags}") == 0) {
                    project.config.build_flags = value;
                } else if (key.compare("config{test_flags}") == 0) {
                    project.config.test_flags = value;
                } else {
                    throw std::runtime_error("Invalid configuration at line " + std::to_string(line_number) + " for key '" + key + "'");
                }
            }

            if (project.authors.size() == 0) {
                throw std::runtime_error("At least one author is required in 'project.cfg'");
            }
            if (project.platforms.size() == 0) {
                throw std::runtime_error("At least one platform is required in 'project.cfg'");
            }

            return project;
        } else {
            throw std::runtime_error("'project.cfg' missing!");
        }
    }

    string convert_model_to_cfg(const Project& project, const bool add_disclaimer_text) {
        const string disclaimer_text{ std::string("; Since a rudimentary INI parser is used, ensure that the actual `key` and")
            + "\n; `value` pairs follow the same `key` and `value` format in this file which was"
            + "\n; provided while creation of the project. Also, ensure that each pair is"
            + "\n; contained within a single line." };

        const string base_text{ std::string("name=") + project.name 
            + "\ndescription=" + project.description
            + "\nversion=" + project.version };
        
        string authors_text{ std::string("; `authors` is always an array even if there is only one entity. At least one")
            + "\n; author is required." };

        for (const Author &author: project.authors) {
            authors_text += std::string("\nauthors[]=") + author.name + AUTHOR_DELIMITER + author.email_id;
        }

        string platforms_text{ std::string("; `platforms` is always an array even if there is only one supported platform,")
            + "\n; and values can be any of 'bsd', 'linux', 'macos', 'unix', `windows`. At least"
            + "\n; one platform is required." };
        
        for (const Platform &platform: project.platforms) {
            platforms_text += std::string("\nplatforms[]=") + platform_to_string(platform);
        }

        const string config_text{ std::string("; `config` contains all the set of attributes required to compile, test and")
            + "\n; build the project."
            + "\nconfig{cpp_standard}=" + project.config.cpp_standard
            + "\nconfig{safety_flags}=" + project.config.safety_flags
            + "\nconfig{compile_time_flags}=" + project.config.compile_time_flags
            + "\nconfig{build_flags}=" + project.config.build_flags
            + "\nconfig{test_flags}=" + project.config.test_flags };

        return (add_disclaimer_text ? (disclaimer_text + "\n\n") : "") 
            + base_text
            + "\n\n"
            + "type=" + project_type_to_string(project.project_type)
            + "\n\n"
            + authors_text
            + "\n\n"
            + platforms_text
            + "\n\n"
            + config_text
            + "\n";
    }
}
