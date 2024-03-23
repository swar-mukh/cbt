#include "workspace/env_parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

#include "workspace/util.hpp"

namespace workspace::env_parser {
    namespace fs = std::filesystem;

    using std::cerr;
    using std::cout;
    using std::endl;
    using std::ifstream;
    using std::string;

    void set(const string key, const string value) {
        if (key.compare("a_bool_entry") == 0) {
            env_values["a_bool_entry"] = value.compare("true") == 0 ? true : false;
        } else if (key.compare("an_int_entry") == 0) {
            env_values["an_int_entry"] = std::stoi(value);
        } else if (key.compare("a_float_entry") == 0) {
            env_values["a_float_entry"] = std::stof(value);
        } else if (key.compare("a_string_entry") == 0) {
            env_values["a_string_entry"] = value;
        }
    }

    void read_template() {
        const string template_file_name{ "environments/.env.template" };

        if (fs::exists(template_file_name)) {
            ifstream env_file(template_file_name);
            string line;

            while (std::getline(env_file, line)) {
                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, string("="));
                env_template[key] = value;
            }
        } else {
            cerr << "Template environment file 'environments/.env.template' missing!" << endl;
        }
    }

    ALLOWED_ENV_DATA_TYPES get_env(const string key) {
        const string return_type = env_template[key];

        if (!env_values.contains(key)) {
            return std::monostate();
        } else {
            return env_values[key];
        }
    }

    void read_env_file(const string env) {
        const string env_file_name{ "environments/" + env + ".env" };

        if (fs::exists(env_file_name)) {
            ifstream env_file(env_file_name);
            string line;

            while (std::getline(env_file, line)) {
                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, string("="));
                set(key, value);
            }
        } else {
            cerr << "No such environment '" << env << "'!" << endl;
        }
    }
}
