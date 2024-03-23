#include "workspace/env_parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>

namespace workspace::env_parser {
    namespace fs = std::filesystem;

    using std::cerr;
    using std::cout;
    using std::endl;
    using std::ifstream;
    using std::string;

    void set(string key, string value) {
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
                const string env_entry = string(line);
                const int delimiter = env_entry.find("=");

                const string key = env_entry.substr(0, delimiter);
                const string value = env_entry.substr(delimiter + 1);

                env_template[key] = value;
            }
        } else {
            cerr << "Template environment file 'environments/.env.template' missing!" << endl;
        }   
    }

    VARIANT_TYPE get_env(string key) {
        const string return_type = env_template[key];

        if (!env_values.contains(key)) {
            return std::monostate();
        } else {
            return env_values[key];
        }
    }

    void read_env_file(string env) {
        const string env_file_name{ "environments/" + env + ".env" };

        if (fs::exists(env_file_name)) {
            ifstream env_file(env_file_name);
            string line;

            while (std::getline(env_file, line)) {
                const string env_entry = string(line);
                const int delimiter = env_entry.find("=");

                const string key = env_entry.substr(0, delimiter);
                const string value = env_entry.substr(delimiter + 1);

                set(key, value);
            }
        } else {
            cerr << "No such environment '" << env << "'!" << endl;
        }
    }
}
