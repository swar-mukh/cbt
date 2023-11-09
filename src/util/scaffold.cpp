#include "util/scaffold.hpp"

#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace util::scaffold {
    namespace fs = std::experimental::filesystem;
    
    using std::cout;
    using std::endl;
    using std::ofstream;
    using std::string;
    using std::stringstream;
    using std::vector;

    const string GITIGNORE = R"(
    build
    test
    )";

    const string README_MD = R"(
    # Project name

    This project was made using `cbt`

    ## Setup

    Discuss your setup here

    ## Code of contribution

    Discuss rules of engagement here
    )";

    const string SAMPLE_HPP = R"(
    #ifndef SAMPLE
    #define SAMPLE

    #include <iostream>
    #include <vector>

    namespace sample {
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

    #include "sample.hpp"

    int main(const int argc, char *argv[], char *envp[]) {
        std::vector<std::string> args(argv, argv + argc);
        std::map<std::string, std::string> env;

        while (*envp) {
            const std::string env_variable = std::string(*envp++);
            const int position_of_equal_to_symbol = env_variable.find("=");

            const std::string key = env_variable.substr(0, position_of_equal_to_symbol);
            const std::string value = env_variable.substr(position_of_equal_to_symbol + 1);

            env[key] = value;
        }
        
        std::cout << "args[0]: " << args[0] << std::endl;
        std::cout << "env[\"HOME\"]: " << env["HOME"] << std::endl;

        std::cout << "Sum of 2 and 3 is: " << sample::sum(2, 3) << std::endl;

        return EXIT_SUCCESS;
    }
    )";

    const string SAMPLE_CPP = R"(
    #include "sample.hpp"

    #include <iostream>
    #include <vector>

    namespace sample {
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

    const string PROJECT_INI = R"(
    ; Since a rudimentary INI parser is used, ensure that the actual `key` and `value` pairs
    ; follow the same `key` and `value` format in this file which was provided while creation
    ; of the project. Also, ensure that each pair is contained within a single line.

    name=my-project
    description=A sample project that does so and so

    version=2023-11-09

    ; `authors` is always an array even if there is only one entity, At least one author is required.
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

        ofstream file_to_write(full_path);
        file_to_write << get_predefined_text_content(file_name);
        file_to_write.close();

        cout << "CREATE " << full_path << endl;
    }

    bool create_directory(const string project_name, const string sub_directory) {
        const string full_path = project_name + "/" + sub_directory;
        
        if (fs::create_directory(full_path)) {
            cout << "CREATE " << full_path << endl;
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
        } else if (file_name.compare("headers/sample.hpp") == 0) {
            return __remove_raw_literal_indentations(SAMPLE_HPP);
        } else if (file_name.compare("src/main.cpp") == 0) {
            return __remove_raw_literal_indentations(MAIN_CPP);
        } else if (file_name.compare("src/sample.cpp") == 0) {
            return __remove_raw_literal_indentations(SAMPLE_CPP);
        } else if (file_name.compare("README.md") == 0) {
            return __remove_raw_literal_indentations(README_MD);
        } else if (file_name.compare("project.ini") == 0) {
            return __remove_raw_literal_indentations(PROJECT_INI);
        } else {
            return "";
        }
    }
}
