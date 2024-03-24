#include "workspace/env_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <variant>

#include "workspace/util.hpp"

namespace workspace::env_manager {
    namespace fs = std::filesystem;

    using std::cerr;
    using std::cout;
    using std::endl;
    using std::ifstream;
    using std::string;
    
    template<typename T>
    T __parse_value_for_key(const string key, const string value) {
        const string name = typeid(T).name();
        string data_type{ "" };

        if (name.compare("b") == 0 || name.compare("bool") == 0) {
            data_type = "bool";
        } else if (name.compare("i") == 0 || name.compare("int") == 0) {
            data_type = "int";
        } else if (name.compare("f") == 0 || name.compare("float") == 0) {
            data_type = "float";
        } else {
            data_type = "string";
        }

        try {
            if (data_type.compare("bool") == 0) {
                if (value.compare("true") == 0 || value.compare("false") == 0) {
                    return value.compare("true") == 0 ? true : false;
                } else {
                    throw std::invalid_argument("");
                }
            } else {
                return data_type.compare("int") == 0 ? std::stoi(value) : std::stof(value);
            }
        } catch (const std::invalid_argument &e) {
            throw std::invalid_argument("Could not parse value for '" + key + "' to '" + data_type + "' type.");
        } catch (const std::out_of_range &e) {
            throw std::invalid_argument("Value for '" + key + "' falls out of range of '" + data_type + "' type.");
        }
    }

    void set(const string key, const string value) {
        if (key.compare("a_bool_entry") == 0) {
            env_values["a_bool_entry"] = __parse_value_for_key<bool>(key, value);
        } else if (key.compare("an_int_entry") == 0) {
            env_values["an_int_entry"] = __parse_value_for_key<int>(key, value);
        } else if (key.compare("a_float_entry") == 0) {
            env_values["a_float_entry"] = __parse_value_for_key<float>(key, value);
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
                std::erase(line, '\r');

                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, string("="));

                if (value.compare("bool") != 0 && value.compare("int") != 0 && value.compare("float") != 0 && value.compare("string") != 0) {
                    throw std::domain_error("Unsupported data type '" + value + "' for key '" + key + "'");
                } else {
                    env_template[key] = value;
                }
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
                std::erase(line, '\r');

                const auto [key, value] = workspace::util::get_key_value_pair_from_line(line, string("="));

                if (!env_template.contains(key)) {
                    throw std::domain_error("Key '" + key + "' absent in 'environments/.env.template'");
                } else {
                    set(key, value);
                }
            }
        } else {
            cerr << "No such environment '" << env << "'!" << endl;
        }
    }

    void prepare_env(std::map<string, string> env) {
        try {
            read_template();

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
