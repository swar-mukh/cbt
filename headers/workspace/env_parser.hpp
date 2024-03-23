#ifndef WORKSPACE_ENV_PARSER
#define WORKSPACE_ENV_PARSER

#include <map>
#include <string>
#include <variant>

namespace workspace::env_parser {
    using std::map;
    using std::string;

    using VARIANT_TYPE = std::variant<std::monostate, bool, int, float, string>;

    static map<string, string> env_template;
    static map<string, VARIANT_TYPE> env_values;

    VARIANT_TYPE get_env(string key);
    void set(string key, string value);

    void read_template();
    void read_env_file(string env);
}

#endif