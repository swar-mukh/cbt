#include "workspace/scaffold.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "workspace/util.hpp"

namespace workspace::scaffold {
    namespace fs = std::filesystem;
    
    using std::cout;
    using std::endl;
    using std::ofstream;
    using std::string;
    using std::stringstream;
    using std::vector;

    const string GITIGNORE = R"(
    build
    test
    environments/*.env
    )";

    const string README_MD = R"(
    # Project name

    This project was made using `cbt`

    ## Setup

    Discuss your setup here

    ## Code of contribution

    Discuss rules of engagement here
    )";

    const string CBT_TOOLS_ENV_MANAGER_HPP = R"(
    #ifndef CBT_TOOLS_ENV_MANAGER
    #define CBT_TOOLS_ENV_MANAGER

    #include <map>
    #include <string>
    #include <variant>

    namespace cbt_tools::env_manager {
        using std::string;

        using ALLOWED_ENV_DATA_TYPES = std::variant<bool, int, float, string>;

        ALLOWED_ENV_DATA_TYPES get_env(const string key);
        void prepare_env(std::map<string, string> env);
    }

    #endif
    )";

    const string CBT_TOOLS_ENV_MANAGER_CPP = R"(
    #include "cbt_tools/env_manager.hpp"

    #include <cstdlib>
    #include <filesystem>
    #include <functional>
    #include <fstream>
    #include <iostream>
    #include <map>
    #include <string>
    #include <stdexcept>
    #include <variant>

    #include "cbt_tools/utils.hpp"

    namespace cbt_tools::env_manager {
        namespace fs = std::filesystem;

        using std::cerr;
        using std::cout;
        using std::endl;
        using std::ifstream;
        using std::string;

        const string DELIMITER = string("=");

        std::map<string, string> env_template;
        std::map<string, ALLOWED_ENV_DATA_TYPES> env_values;

        // Add/update necessary parsers as required
        std::map<string, std::function<ALLOWED_ENV_DATA_TYPES(const string, const string)>> PARSERS{
            { "bool", [](const string key, const string value) {
                if (value.compare("true") == 0 || value.compare("false") == 0) {
                    return value.compare("true") == 0 ? true : false;
                } else {
                    throw std::invalid_argument("Could not parse value for '" + key + "' to 'bool' type. Expected either 'true' or 'false'.");
                }
            }},
            { "int", [](const string key, const string value) {
                try {
                    return std::stoi(value);
                } catch (const std::invalid_argument &e) {
                    throw std::invalid_argument("Could not parse value for '" + key + "' to 'int' type.");
                } catch (const std::out_of_range &e) {
                    throw std::invalid_argument("Value for '" + key + "' falls out of range of 'int' type.");
                }
            }},
            { "float", [](const string key, const string value) {
                try {
                    return std::stof(value);
                } catch (const std::invalid_argument &e) {
                    throw std::invalid_argument("Could not parse value for '" + key + "' to 'float' type.");
                } catch (const std::out_of_range &e) {
                    throw std::invalid_argument("Value for '" + key + "' falls out of range of 'float' type.");
                }
            }},
            { "string", []([[maybe_unused]] const string _, const string value) { return value; } },
        };

        void __set(const string key, const string value) {
            // Use the below conditional checks and keep adding the keys
            // that have been defined in 'environments/.env.template' file

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

        // You would typically not need to touch this function
        ALLOWED_ENV_DATA_TYPES get_env(const string key) {
            if (env_values.contains(key)) {
                return env_values[key];
            } else {
                throw std::invalid_argument("Trying to access invalid key '" + key + "'");
            }
        }
        
        // You would typically not need to touch this function
        void __read_template_file() {
            const string template_file_name{ "environments/.env.template" };

            if (fs::exists(template_file_name)) {
                ifstream env_file(template_file_name);
                string line;

                while (std::getline(env_file, line)) {
                    std::erase(line, '\r');

                    const auto [key, value] = cbt_tools::utils::get_key_value_pair_from_line(line, DELIMITER);

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

        // You would typically not need to touch this function
        void __read_env_file(const string env) {
            const string env_file_name{ "environments/" + env + ".env" };

            ifstream env_file(env_file_name);
            string line;

            while (std::getline(env_file, line)) {
                std::erase(line, '\r');

                const auto [key, value] = cbt_tools::utils::get_key_value_pair_from_line(line, DELIMITER);

                if (!env_template.contains(key)) {
                    throw std::domain_error("Key '" + key + "' absent in 'environments/.env.template'");
                } else {
                    __set(key, value);
                }
            }
        }

        // You would typically not need to touch this function
        void prepare_env(std::map<string, string> env) {
            try {
                __read_template_file();

                if (env["env"].length() != 0) {
                    __read_env_file(env["env"]);
                } else {
                    __read_env_file("local");
                }
            } catch (const std::exception &e) {
                cerr << endl << "Exception: " << e.what() << endl << endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
    )";

    const string CBT_TOOLS_UTILS_HPP = R"(
    #ifndef CBT_TOOLS_UTILS
    #define CBT_TOOLS_UTILS

    #include <string>
    #include <tuple>

    namespace cbt_tools::utils {
        using std::string;

        std::tuple<string, string> get_key_value_pair_from_line(const string line, const string delimiter);
    }

    #endif
    )";

    const string CBT_TOOLS_UTILS_CPP = R"(
    #include "cbt_tools/utils.hpp"

    #include <string>
    #include <tuple>

    namespace cbt_tools::utils {
        using std::string;

        std::tuple<string, string> get_key_value_pair_from_line(const string line, const string delimiter) {
            const int delimiter_position = line.find(delimiter);

            const string key = line.substr(0, delimiter_position);
            const string value = line.substr(delimiter_position + 1);

            return std::make_tuple(key, value);
        }
    }
    )";
    
    const string SAMPLE_HPP = R"(
    #ifndef @GUARD
    #define @GUARD

    #include <iostream>
    #include <vector>

    namespace @NAMESPACE {
        int sum(const int a, const int b);

        enum class Sex {
            MALE,
            FEMALE,
            NON_BINARY
        };

        struct Person {
            std::string first_name;
            std::string last_name;
            Sex sex;

            friend std::ostream& operator<<(std::ostream& out, const Person& person);
        };

        struct Employee {
            std::string id;
            std::string first_name;
            std::string last_name;
            Sex sex;

            friend std::ostream& operator<<(std::ostream& out, const Employee& employee);
        };

        class SampleCompany {
        public:
            SampleCompany(const std::string location);
            bool fire(const std::string employee_id, const std::string reason);
            std::string get_location() const;
            Employee hire(const Person person);
            bool is_candidate_eligible(const Person person) const;
            std::vector<Employee> list_absentees() const;
            friend std::ostream& operator<<(std::ostream& out, const SampleCompany& company);
        
        private:
            Employee founder;
            std::vector<Employee> employees;
            std::string location;
        };
    }

    #endif
    )";

    const string MAIN_CPP = R"(
    #include <iostream>
    #include <map>

    #include "cbt_tools/env_manager.hpp"
    #include "cbt_tools/utils.hpp"

    #include "sample.hpp"

    int main(const int argc, char *argv[], char *envp[]) {
        std::vector<std::string> args(argv, argv + argc);
        std::map<std::string, std::string> env;

        while (*envp) {
            const auto [key, value] = cbt_tools::utils::get_key_value_pair_from_line(std::string(*envp++), std::string("="));
            env[key] = value;
        }
        
        cbt_tools::env_manager::prepare_env(env);

        std::cout << "args[0]: " << args[0] << std::endl;
        std::cout << "env[\"HOME\"]: " << env["HOME"] << std::endl;

        const std::string sample_env_key { "a_float_entry" };
        auto env_value = cbt_tools::env_manager::get_env(sample_env_key);

        if (std::holds_alternative<float>(env_value)) {
            const float value = std::get<float>(env_value);
            std::cout << "Env. value of '" << sample_env_key << "' is: " << value << std::endl;
        }

        std::cout << "Sum of 2 and 3 is: " << sample::sum(2, 3) << std::endl;

        return EXIT_SUCCESS;
    }
    )";

    const string SAMPLE_CPP = R"(
    #include "@FILE_NAME"

    #include <iostream>
    #include <vector>

    namespace @NAMESPACE {
        int sum(const int a, const int b) {
            return a + b;
        }

        std::ostream& operator<<(std::ostream& out, const Person& person) {
            out << person.first_name << std::endl;
            return out;
        }

        std::ostream& operator<<(std::ostream& out, const Employee& employee) {
            out << employee.id << std::endl;
            return out;
        }

        SampleCompany::SampleCompany(std::string location): location(location){}

        std::string SampleCompany::get_location() const {
            return this->location;
        }

        std::ostream& operator<<(std::ostream& out, const SampleCompany& company) {
            out << company.get_location() << std::endl;
            return out;
        }
    }
    )";

    const string LICENSE_TXT = R"(
    This is a sample license file.

    Add actual content in this file.
    )";

    const string ENV_TEMPLATE = R"(
    a_bool_entry=bool
    an_int_entry=int
    a_float_entry=float
    a_string_entry=string
    )";

    const string ENV_FILE = R"(
    a_bool_entry=true
    an_int_entry=123
    a_float_entry=4.56
    a_string_entry=Hello there!
    )";

    const string ROADMAP_MD = R"(
    # Sample Roadmap

    - [X] Get legal documents
    - [ ] Setup infrastructure
    - [ ] Develop login feature:
        - [X] Model the data
        - [ ] Sanitise data
        - [ ] Persist in database
    - [ ] Setup media driver:
        - [ ] Get necessary interface descriptions
        - [ ] Perform R/W
        - [ ] Subject code to thorough testing
    )";

    const string PROJECT_CFG = R"(
    ; Since a rudimentary INI parser is used, ensure that the actual `key` and `value` pairs
    ; follow the same `key` and `value` format in this file which was provided while creation
    ; of the project. Also, ensure that each pair is contained within a single line.

    name=my-project
    description=A sample project that does so and so

    version=2023-11-09

    ; `authors` is always an array even if there is only one entity. At least one author is required.
    authors[]=Sample LName <sample_lname@domain.tld>
    authors[]=Another MName LName <another_nmane_lname@domain.tld>

    ; `platforms` is always an array even if there is only one supported platform and 
    ; values can be any of 'linux', 'macos', 'unix', `windows`. At least one platform is required.
    platforms[]=linux
    platforms[]=macos
    platforms[]=unix
    platforms[]=windows
    )";

    string __remove_raw_literal_indentations(const string raw_literal) {
        string line, final_string;
        stringstream stream(raw_literal);
        vector<string> lines;

        while (getline(stream, line)) {
            lines.push_back(line);
        }

        lines.erase(lines.begin());
        lines.pop_back();

        for (auto& nline: lines) {
            final_string += (nline.size() > 4 ? nline.substr(4) : nline) + "\n";
        }

        return final_string;
    }

    void create_file(const string project_name, const string file_name) {
        const string full_path = project_name +  "/" + file_name;

        if (fs::exists(full_path)) {
            cout << std::right << std::setw(8) <<  "SKIP " << full_path << endl;
        } else {
            if (!fs::exists(full_path.substr(0, full_path.find_last_of("/")))) {
                create_directory(".", full_path.substr(0, full_path.find_last_of("/")), true);
            }

            ofstream file_to_write(full_path);
            file_to_write << get_predefined_text_content(file_name);
            file_to_write.close();

            cout << std::right << std::setw(8) << "CREATE " << full_path << endl;
        }
    }

    bool create_directory(const string project_name, const string sub_directory, bool multi_directory) {
        string full_path =( project_name + "/" + sub_directory + (sub_directory.length() != 0 ? "/" : ""));

        if (full_path.starts_with("././")) {
            full_path = full_path.replace(0, 4, "./");
        }
        
        const bool result{ multi_directory ? fs::create_directories(full_path) : fs::create_directory(full_path) };

        if (result) {
            cout << std::right << std::setw(8) << "DIR " << full_path << endl;
            return true;
        } else {
            return false;
        }
    }

    string get_predefined_text_content(const string file_name) {
        if (file_name.compare(".gitignore") == 0) {
            return __remove_raw_literal_indentations(GITIGNORE);
        } else if (file_name.compare("docs/LICENSE.txt") == 0) {
            return __remove_raw_literal_indentations(LICENSE_TXT);
        } else if (file_name.compare("docs/Roadmap.md") == 0) {
            return __remove_raw_literal_indentations(ROADMAP_MD);
        } else if (file_name.compare("environments/.env.template") == 0) {
            return __remove_raw_literal_indentations(ENV_TEMPLATE);
        } else if (file_name.starts_with("environments/") && file_name.ends_with(".env")) {
            return __remove_raw_literal_indentations(ENV_FILE);
        } else if (file_name.compare("headers/cbt_tools/env_manager.hpp") == 0) {
            return __remove_raw_literal_indentations(CBT_TOOLS_ENV_MANAGER_HPP);
        } else if (file_name.compare("headers/cbt_tools/utils.hpp") == 0) {
            return __remove_raw_literal_indentations(CBT_TOOLS_UTILS_HPP);
        } else if (file_name.compare("src/cbt_tools/env_manager.cpp") == 0) {
            return __remove_raw_literal_indentations(CBT_TOOLS_ENV_MANAGER_CPP);
        } else if (file_name.compare("src/cbt_tools/utils.cpp") == 0) {
            return __remove_raw_literal_indentations(CBT_TOOLS_UTILS_CPP);
        } else if (file_name.ends_with(".hpp")) {
            const string text{ __remove_raw_literal_indentations(SAMPLE_HPP) };
            const auto [stemmed_name, guard_name, namespace_name] = workspace::util::get_qualified_names(file_name);
            
            const string with_guard = std::regex_replace(text, GUARD_R, guard_name);
            const string with_import = std::regex_replace(with_guard, IMPORT_R, stemmed_name + ".hpp");
            const string final_text = std::regex_replace(with_import, NAMESPACE_R, namespace_name);
            
            return final_text;
        } else if (file_name.compare("src/main.cpp") == 0) {
            return __remove_raw_literal_indentations(MAIN_CPP);
        } else if (file_name.ends_with(".cpp")) {
            const string text{ __remove_raw_literal_indentations(SAMPLE_CPP) };
            const auto [stemmed_name, guard_name, namespace_name] = workspace::util::get_qualified_names(file_name);
            
            const string with_import = std::regex_replace(text, IMPORT_R, stemmed_name + ".hpp");
            const string final_text = std::regex_replace(with_import, NAMESPACE_R, namespace_name);
            
            return final_text;
        } else if (file_name.compare("README.md") == 0) {
            return __remove_raw_literal_indentations(README_MD);
        } else if (file_name.compare("project.cfg") == 0) {
            return __remove_raw_literal_indentations(PROJECT_CFG);
        } else {
            return "";
        }
    }

    bool is_command_invoked_from_workspace() {
        return fs::exists("project.cfg") && fs::exists("headers/") &&fs::exists("src/");
    }
}
