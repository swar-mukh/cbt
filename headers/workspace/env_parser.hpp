#ifndef WORKSPACE_ENV_PARSER
#define WORKSPACE_ENV_PARSER

#include <map>
#include <string>
#include <variant>

namespace workspace::env_parser {
    using std::map;
    using std::string;

    using ALLOWED_ENV_DATA_TYPES = std::variant<std::monostate, bool, int, float, string>;

    static map<string, string> env_template;
    static map<string, ALLOWED_ENV_DATA_TYPES> env_values;

    ALLOWED_ENV_DATA_TYPES get_env(const string key);
    void set(const string key, const string value);

    void read_template();
    void read_env_file(const string env);
}

#endif