#include "workspace/env_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <variant>

#include "workspace/util.hpp"

namespace {
    using namespace workspace::env_manager;

    namespace fs = std::filesystem;

    using std::cerr;
    using std::cout;
    using std::endl;
    using std::ifstream;
    using std::string;

    const string DELIMITER{ "=" };

    std::map<string, string> env_template;
    std::map<string, ALLOWED_ENV_DATA_TYPES> env_values;

    std::map<string, std::function<ALLOWED_ENV_DATA_TYPES(const string&, const string&)>> PARSERS{
        { "bool", [](const string& key, const string& value) {
            if (value.compare("true") == 0 || value.compare("false") == 0) {
                return value.compare("true") == 0 ? true : false;
            } else {
                throw std::invalid_argument("Could not parse value for '" + key + "' to 'bool' type. Expected either 'true' or 'false'.");
            }
        }},
        { "int", [](const string& key, const string& value) {
            try {
                return std::stoi(value);
            } catch (const std::invalid_argument &e) {
                throw std::invalid_argument("Could not parse value for '" + key + "' to 'int' type.");
            } catch (const std::out_of_range &e) {
                throw std::invalid_argument("Value for '" + key + "' falls out of range of 'int' type.");
            }
        }},
        { "float", [](const string& key, const string& value) {
            try {
                return std::stof(value);
            } catch (const std::invalid_argument &e) {
                throw std::invalid_argument("Could not parse value for '" + key + "' to 'float' type.");
            } catch (const std::out_of_range &e) {
                throw std::invalid_argument("Value for '" + key + "' falls out of range of 'float' type.");
            }
        }},
        { "string", []([[maybe_unused]] const string& _, const string& value) { return value; } },
    };

    void set_kv(const string& key, const string& value) {
        if (key.compare("a_bool_entry") == 0) {
            env_values["a_bool_entry"] = PARSERS["bool"](key, value);
        } else if (key.compare("an_int_entry") == 0) {
            env_values["an_int_entry"] = PARSERS["int"](key, value);
        } else if (key.compare("a_float_entry") == 0) {
            env_values["a_float_entry"] = PARSERS["float"](key, value);
        } else if (key.compare("a_string_entry") == 0) {
            env_values["a_string_entry"] = PARSERS["string"](key, value);
        }
    }

    void read_template_file() {
        const string template_file_name{ "environments/.env.template" };

        if (fs::exists(template_file_name)) {
            ifstream env_file(template_file_name);
            string line;

            while (std::getline(env_file, line)) {
                std::erase(line, '\r');

                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, DELIMITER);

                if (PARSERS.contains(value)) {
                    env_template[key] = value;
                } else {
                    throw std::domain_error("Unsupported data type '" + value + "' for key '" + key + "'");
                }
            }
        } else {
            cerr << "Template environment file 'environments/.env.template' missing!" << endl;
        }
    }

    void read_env_file(const string& env) {
        const string env_file_name{ "environments/" + env + ".env" };

        ifstream env_file(env_file_name);
        string line;

        while (std::getline(env_file, line)) {
            std::erase(line, '\r');

            const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, DELIMITER);

            if (!env_template.contains(key)) {
                throw std::domain_error("Key '" + key + "' absent in 'environments/.env.template'");
            } else {
                set_kv(key, value);
            }
        }
    }
}

namespace workspace::env_manager {
    ALLOWED_ENV_DATA_TYPES get_env(const string& key) {
        if (env_values.contains(key)) {
            return env_values[key];
        } else {
            throw std::invalid_argument("Trying to access invalid key '" + key + "'");
        }
    }
    
    void prepare_env(std::map<string, string>& env) {
        try {
            read_template_file();

            if (env["env"].length() != 0) {
                read_env_file(env["env"]);
            } else {
                read_env_file("local");
            }
        } catch (const std::exception &e) {
            cerr << endl << "Exception: " << e.what() << endl << endl;
            std::exit(EXIT_FAILURE);
        }
    }
}
