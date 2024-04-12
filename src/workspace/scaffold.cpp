#include "workspace/scaffold.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "assets/scaffold_texts.hpp"
#include "workspace/util.hpp"

namespace workspace::scaffold {
    namespace fs = std::filesystem;
    
    using namespace assets::scaffold_texts;
    using std::cout;
    using std::endl;
    using std::ofstream;
    using std::string;
    using std::stringstream;
    using std::vector;

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
