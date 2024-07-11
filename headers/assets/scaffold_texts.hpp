#ifndef ASSETS_SCAFFOLD_TEXTS
#define ASSETS_SCAFFOLD_TEXTS

#include <string>

namespace assets::scaffold_texts {
    using std::string;

    const string GITIGNORE = R"(
    .internals
    build
    environments/*.env
    )";

    const string README_MD = R"(
    # @PROJECT_NAME

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

        ALLOWED_ENV_DATA_TYPES get_env(const string& key);
        void prepare_env(std::map<string, string>& env);
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

    namespace {
        using namespace cbt_tools::env_manager;

        namespace fs = std::filesystem;

        using std::cerr;
        using std::cout;
        using std::endl;
        using std::ifstream;
        using std::string;

        const string DELIMITER{ "=" };

        std::map<string, string> env_template;
        std::map<string, ALLOWED_ENV_DATA_TYPES> env_values;

        // Add/update necessary parsers as required
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
                } catch (const std::invalid_argument& e) {
                    throw std::invalid_argument("Could not parse value for '" + key + "' to 'int' type.");
                } catch (const std::out_of_range& e) {
                    throw std::invalid_argument("Value for '" + key + "' falls out of range of 'int' type.");
                }
            }},
            { "float", [](const string& key, const string& value) {
                try {
                    return std::stof(value);
                } catch (const std::invalid_argument& e) {
                    throw std::invalid_argument("Could not parse value for '" + key + "' to 'float' type.");
                } catch (const std::out_of_range& e) {
                    throw std::invalid_argument("Value for '" + key + "' falls out of range of 'float' type.");
                }
            }},
            { "string", []([[maybe_unused]] const string& _, const string& value) { return value; } },
        };

        void set(const string& key, const string& value) {
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
        void read_template_file() {
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
        void read_env_file(const string& env) {
            const string env_file_name{ "environments/" + env + ".env" };

            ifstream env_file(env_file_name);
            string line;

            while (std::getline(env_file, line)) {
                std::erase(line, '\r');

                const auto [key, value] = cbt_tools::utils::get_key_value_pair_from_line(line, DELIMITER);

                if (!env_template.contains(key)) {
                    throw std::domain_error("Key '" + key + "' absent in 'environments/.env.template'");
                } else {
                    set(key, value);
                }
            }
        }
    }

    namespace cbt_tools::env_manager {
        // You would typically not need to touch this function
        ALLOWED_ENV_DATA_TYPES get_env(const string& key) {
            if (env_values.contains(key)) {
                return env_values[key];
            } else {
                throw std::invalid_argument("Trying to access invalid key '" + key + "'");
            }
        }

        // You would typically not need to touch this function
        void prepare_env(std::map<string, string>& env) {
            try {
                read_template_file();

                if (env["env"].length() != 0) {
                    read_env_file(env["env"]);
                } else {
                    read_env_file("local");
                }
            } catch (const std::exception& e) {
                cerr << endl << "Exception: " << e.what() << endl << endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
    )";

    const string CBT_TOOLS_TEST_HARNESS_HPP = R"(
    #ifndef CBT_TOOLS_TEST_HARNESS
    #define CBT_TOOLS_TEST_HARNESS

    #include <functional>
    #include <iomanip>
    #include <iostream>
    #include <string>
    #include <tuple>
    #include <vector>

    namespace cbt_tools::test_harness {
        // Note: Edit this parent class *only if* the harness provided is not upto your requirements
        class TestSuite {
        public:
            virtual void add_test_case(const std::string& title, std::function<void()> test_case) final {
                test_cases.push_back(std::make_tuple(title, test_case));
            }
            virtual void run() final {
                setup();
                
                for (const auto& test_case: test_cases) {
                    before_each();

                    const auto [title, test_fn] = test_case;
                            
                    std::cout << std::right << std::setw(8) << "RUN " << title << std::endl;
                    test_fn();

                    after_each();
                }
                
                teardown();
            }

        private:
            virtual void setup() = 0;
            virtual void before_each() = 0;
            virtual void after_each() = 0;
            virtual void teardown() = 0;

            std::vector<std::tuple<std::string, std::function<void()>>> test_cases;
        };
    }

    #endif
    )";

    const string CBT_TOOLS_UTILS_HPP = R"(
    #ifndef CBT_TOOLS_UTILS
    #define CBT_TOOLS_UTILS

    #include <string>
    #include <tuple>

    namespace cbt_tools::utils {
        using std::string;

        std::tuple<string, string> get_key_value_pair_from_line(const string& line, const string& delimiter);
    }

    #endif
    )";

    const string CBT_TOOLS_UTILS_CPP = R"(
    #include "cbt_tools/utils.hpp"

    #include <string>
    #include <tuple>

    namespace cbt_tools::utils {
        using std::string;

        std::tuple<string, string> get_key_value_pair_from_line(const string& line, const string& delimiter) {
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
        void a_function();

        int sum(const int a, const int b);

        enum class Sex {
            MALE,
            FEMALE
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
            explicit SampleCompany(const std::string& name, const std::string& location, const Employee& founder);

            std::string get_name() const;
            std::string get_location() const;
            Employee get_founder() const;
            int strength() const;

            bool is_candidate_eligible(const Person& person) const;
            void hire(const Person& person);
            bool fire(const std::string& employee_id, const std::string& reason);
            
            std::vector<Employee> list_absentees() const;

            friend std::ostream& operator<<(std::ostream& out, const SampleCompany& company);
        
        private:
            std::string name;
            std::string location;
            Employee founder;
            std::vector<Employee> employees;
        };
    }

    #endif
    )";

    const string MAIN_CPP = R"(
    #include <iostream>
    #include <map>
    #include <vector>

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
        std::cout << "env[\"HOME\"]: " << env["HOME"] << std::endl << std::endl;

        const std::string sample_env_key { "a_float_entry" };
        auto env_value = cbt_tools::env_manager::get_env(sample_env_key);

        if (std::holds_alternative<float>(env_value)) {
            const float value = std::get<float>(env_value);
            std::cout << "Env. value of '" << sample_env_key << "' is: " << value << std::endl;
        }

        std::cout << std::endl << "Sum of 2 and 3 is: " << sample::sum(2, 3) << std::endl << std::endl;

        sample::SampleCompany company("MyCompany", "MyLocation", sample::Employee{
            .id{ "#E1" },
            .first_name{ "First" },
            .last_name{ "Name" },
            .sex{ sample::Sex::MALE }
        });

        company.hire(sample::Person{ .first_name{ "F1" }, .last_name{ "L1" }, .sex{ sample::Sex::MALE } });
        company.hire(sample::Person{ .first_name{ "F2" }, .last_name{ "L2" }, .sex{ sample::Sex::FEMALE } });

        std::cout << company;

        return EXIT_SUCCESS;
    }
    )";

    const string SAMPLE_CPP = R"(
    #include "@FILE_NAME"

    #include <iomanip>
    #include <iostream>

    namespace {
        using namespace @NAMESPACE;

        // Define all private functionalities here. These are auxiliary items
        // that will be available only in this file.

        void some_helper() {
            std::cout << "Helper called!";
        }
    }

    namespace @NAMESPACE {
        void a_function() {
            some_helper();
        }

        int sum(const int a, const int b) {
            return a + b;
        }

        std::ostream& operator<<(std::ostream& out, const Person& person) {
            out << "Person{"
                << " first_name: " << std::quoted(person.first_name)
                << ", last_name: " << std::quoted(person.last_name)
                << ", sex: " << (person.sex == Sex::MALE ? 'M' : 'F')
                << " }";
            
            return out;
        }

        std::ostream& operator<<(std::ostream& out, const Employee& employee) {
            out << "Employee{"
                << " id: " << std::quoted(employee.id)
                << ", first_name: " << std::quoted(employee.first_name)
                << ", last_name: " << std::quoted(employee.last_name)
                << ", sex: " << (employee.sex == Sex::MALE ? 'M' : 'F')
                << " }";
            
            return out;
        }

        SampleCompany::SampleCompany(const std::string& name, const std::string& location, const Employee& founder):
            name{ name },
            location{ location },
            founder{ founder } {}

        std::string SampleCompany::get_name() const {
            return this->name;
        }

        std::string SampleCompany::get_location() const {
            return this->location;
        }

        Employee SampleCompany::get_founder() const {
            return this->founder;
        }

        int SampleCompany::strength() const {
            return this->employees.size() + 1;
        }

        void SampleCompany::hire(const Person& person) {
            this->employees.push_back(Employee{
                .id{ "#E" + std::to_string(this->employees.size() + 1) },
                .first_name{ person.first_name },
                .last_name{ person.last_name  },
                .sex{ person.sex }
            });
        }

        std::ostream& operator<<(std::ostream& out, const SampleCompany& company) {
            out << "Company{\n"
                << " name: " << std::quoted(company.get_name()) << "\n"
                << " location: " << std::quoted(company.get_location()) << "\n"
                << " founder: " << company.get_founder() << "\n"
                << " strength: " << company.strength() << "\n"
                << "}" << std::endl;
            
            return out;
        }
    }
    )";

    const string SAMPLE_TEST_CPP = R"(
    #include "cbt_tools/test_harness.hpp"

    #include <cassert>
    #include <functional>
    #include <iostream>
    #include <string>

    #include "@RELATIVE_SRC_FILE_NAME"

    class ScopedTestSuite: public cbt_tools::test_harness::TestSuite {
    private:
        void setup() override {
            // Add necessary code here
        }
        void before_each() override {
            // Add necessary code here
            sample_env_int = 1;
        }
        void after_each() override {
            // Add necessary code here
            sample_env_int = 2;
        }
        void teardown() override {
            // Add necessary code here
        }

        // add your environment variables and functionalities here, if necessary
        int sample_env_int{ 0 };
    };

    int main() {
        ScopedTestSuite test_suite;

        std::cout << std::endl << std::setw(8) << "EXECUTE " << __FILE__ << std::endl << std::endl;

        test_suite.add_test_case("Sum of 5 and 6 is 11", []() {
            assert((sample::sum(5, 6) == 11));
        });
        
        test_suite.add_test_case("Sum of 5 and 6 is not 12", []() {
            assert((sample::sum(5, 6) != 12));
        });

        test_suite.add_test_case("Company foundation strength is 1", []() {
            sample::SampleCompany company("MyCompany", "MyLocation", sample::Employee{
                .id{ "#E1" },
                .first_name{ "First" },
                .last_name{ "Name" },
                .sex{ sample::Sex::MALE }
            });

            assert((company.strength() == 1));
        });

        test_suite.add_test_case("Company's strength is 3 upon hiring of 2 candidates", []() {
            sample::SampleCompany company("MyCompany", "MyLocation", sample::Employee{
                .id{ "#E1" },
                .first_name{ "First" },
                .last_name{ "Name" },
                .sex{ sample::Sex::MALE }
            });

            company.hire(sample::Person{ .first_name{ "F1" }, .last_name{ "L1" }, .sex{ sample::Sex::MALE } });
            company.hire(sample::Person{ .first_name{ "F2" }, .last_name{ "L2" }, .sex{ sample::Sex::FEMALE } });

            assert((company.strength() == 3));
        });
        
        test_suite.run();

        return EXIT_SUCCESS;
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
}

#endif
