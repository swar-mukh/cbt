#ifndef WORKSPACE_ENV_MANAGER
#define WORKSPACE_ENV_MANAGER

#include <map>
#include <string>
#include <variant>

namespace workspace::env_manager {
    using std::string;

    using ALLOWED_ENV_DATA_TYPES = std::variant<bool, int, float, string>;

    ALLOWED_ENV_DATA_TYPES get_env(const string key);
    void prepare_env(std::map<string, string> env);
}

#endif