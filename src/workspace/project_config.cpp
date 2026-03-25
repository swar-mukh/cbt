#include "workspace/project_config.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <set>
#include <string>

#include "workspace/scaffold.hpp"
#include "workspace/util.hpp"

namespace {
    using namespace workspace::project_config;

    const std::set<std::string> VALID_ATTRIBUTES{
        "name",
        "description",
        "version",
        "type",
        "authors[]",
        "platforms[]",
        "config{cpp_standard}",
        "config{safety_flags}",
        "config{compile_time_flags}",
        "config{build_flags}",
        "config{test_flags}",
        "dependencies[]"
    };
    const std::set<std::string> UNSUPPORTED_CPP_STANDARDS{ "c++98", "c++03", "c++11", "c++14" };
    const std::set<std::string> SUPPORTED_CPP_STANDARDS{ "c++17", "c++20", "c++23" };

    bool is_valid_attribute(const std::string& attribute) {
        return VALID_ATTRIBUTES.contains(attribute);
    }

    void validate_required_fields(const Project& project) {
        if (project.name.empty()) {
            throw std::runtime_error("Missing entry 'name' (in 'project.cfg')");
        }

        if (project.description.empty()) {
            throw std::runtime_error("Missing entry 'description' (in 'project.cfg')");
        }

        if (project.version.empty()) {
            throw std::runtime_error("Missing entry 'version' (in 'project.cfg')");
        }

        if (project.authors.size() == 0) {
            throw std::runtime_error("At least one author is required (in 'project.cfg')");
        }

        if (project.platforms.size() == 0) {
            throw std::runtime_error("At least one platform is required (in 'project.cfg')");
        }

        if (project.config.cpp_standard.empty()) {
            throw std::runtime_error("Missing entry 'config{cpp_standard}' (in 'project.cfg')");
        }
    }
}

namespace workspace::project_config {
    namespace fs = std::filesystem;

    using std::string;

    const string AUTHOR_DELIMITER{ ":" };
    const string DELIMITER{ "=" };
    const string LINE_COMMENT{ ";" };

    Project init(const string& project_name, const ProjectType& project_type) {
        return Project{
            .name{ project_name },
            .description{ "Add some description here" },
            .version{ workspace::util::get_ISO_date() },
            .project_type{ project_type },
            .authors{
                { "sample_lname@domain.tld", "Sample LName" },
                { "another_mname_lname@domain.tld", "Another MName LName" }
            },
            .platforms{ Platform::BSD, Platform::LINUX, Platform::MACOS, Platform::UNIX, Platform::WINDOWS },
            .config{
                .cpp_standard{ "c++20" },
                .safety_flags{ "-Wall -Wextra -pedantic" },
                .compile_time_flags{ "-Os -s" },
                .build_flags{ "-O3 -s" },
                .test_flags{ "-g -Og" }
            },
            .dependencies{
                { .name{ "cbt_tools" }, .version{ "2024-08-31" }, .url{ "https://github.com/swar-mukh/cbt_tools" } },
                { .name{ "some_lib" }, .version{ "2025-01-01" }, .url{ "https://gitlab.com/some-user/some_lib" } }
            }
        };
    }

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
        else {
            throw std::domain_error("Unsupported platform '" + platform + "'");
        };
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
            throw std::domain_error("Invalid project type '" + project_type + "'");
        };
    }

    string dependency_to_string(const SurfaceDependency& dependency, const bool exclude_url) {
        return dependency.name + "@" + dependency.version + (exclude_url ? "" : (":" + dependency.url));
    }

    SurfaceDependency parse_dependency(const string& value) {
        const auto [name, rest] = workspace::util::get_key_value_pair_from_line(value, "@");
        const auto [version, url] =  workspace::util::get_key_value_pair_from_line(rest, AUTHOR_DELIMITER);

        return SurfaceDependency{
            .name{ name },
            .version{ version },
            .url{ url }
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

                const string ERROR_LOCATION{ "(in 'project.cfg' at line " + std::to_string(line_number) + ")" };

                if (key.empty()) {
                    throw std::runtime_error("Invalid empty attribute " + ERROR_LOCATION);
                } else if (!is_valid_attribute(key)) {
                    throw std::runtime_error("Unrecognised attribute '" + key + "' " + ERROR_LOCATION);
                } else if (value.empty()) {
                    throw std::runtime_error("Missing entry '" + key + "' " + ERROR_LOCATION);
                }
            
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
                    
                    if (project.authors.contains(email_id)) {
                        throw std::runtime_error("Multiple authors cannot have the same E-mail ID (while resolving '" + email_id + "') " + ERROR_LOCATION);
                    } else {
                        project.authors[email_id] = name;
                    }
                } else if (key.compare("platforms[]") == 0) {
                    project.platforms.insert(string_to_platform(value));
                } else if (key.compare("config{cpp_standard}") == 0) {
                    const std::string cpp_standard{ workspace::util::change_case(value, workspace::util::TextCase::LOWER_CASE) };
                    
                    if(UNSUPPORTED_CPP_STANDARDS.contains(cpp_standard)) {
                        throw std::runtime_error("Minimum supported C++ standard by cbt is 'C++17' " + ERROR_LOCATION);
                    } else if (SUPPORTED_CPP_STANDARDS.contains(cpp_standard)) {
                        project.config.cpp_standard = cpp_standard;
                    } else {
                        throw std::runtime_error("Invalid C++ standard '" + value + "' " + ERROR_LOCATION);
                    }
                } else if (key.compare("config{safety_flags}") == 0) {
                    project.config.safety_flags = value;
                } else if (key.compare("config{compile_time_flags}") == 0) {
                    project.config.compile_time_flags = value;
                } else if (key.compare("config{build_flags}") == 0) {
                    project.config.build_flags = value;
                } else if (key.compare("config{test_flags}") == 0) {
                    project.config.test_flags = value;
                } else if (key.compare("dependencies[]") == 0) {
                    project.dependencies.insert(parse_dependency(value));
                } else {
                    throw std::runtime_error("Invalid configuration at line " + std::to_string(line_number) + " for key '" + key + "'");
                }
            }

            validate_required_fields(project);

            return project;
        } else {
            throw std::runtime_error("'project.cfg' missing!");
        }
    }

    string convert_model_to_cfg(const Project& project, const bool add_disclaimer_text, const bool uncomment_dependencies) {
        const string disclaimer_text{ std::string("; Since a rudimentary INI parser is used, ensure that the actual `key` and")
            + "\n; `value` pairs follow the same `key` and `value` format in this file which was"
            + "\n; provided while creation of the project. Also, ensure that each pair is"
            + "\n; contained within a single line." };

        const string base_text{ std::string("name=") + project.name 
            + "\ndescription=" + project.description
            + "\nversion=" + project.version };
        
        string authors_text{ std::string("; `authors` is always an array even if there is only one entity. At least one")
            + "\n; author is required." };

        for (const auto& [email_id, name]: project.authors) {
            authors_text += std::string("\nauthors[]=") + name + AUTHOR_DELIMITER + email_id;
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
        
        string dependencies_text{ "; add your `dependencies` in the format mentioned below" };

        for (const SurfaceDependency &dependency: project.dependencies) {
            dependencies_text += std::string("\n") + (uncomment_dependencies ? "" : "; ") + "dependencies[]=" + dependency_to_string(dependency, false);
        }

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
            + "\n\n"
            + dependencies_text
            + "\n";
    }
}
