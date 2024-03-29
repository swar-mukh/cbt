#include "workspace/util.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <tuple>

namespace workspace::util {
    using std::string;

    string change_case(string text, const TextCase casing) {
        std::function<int(int)> fn = [casing](char c) {
            return casing == TextCase::LOWER_CASE ? std::tolower(c) : std::toupper(c);
        };

        std::transform(
            text.begin(), 
            text.end(), 
            text.begin(),
            fn
        );

        return text;
    }

    bool does_name_contain_special_characters(const string& text) {
        for (const char ch: text) {
            const auto uch = static_cast<unsigned char>(ch);

            if (!std::isalnum(uch) && !std::isspace(uch) && uch != '_' && uch != '/') {
                return true;
            }
        }

        return false;
    }

    std::tuple<bool, string> is_valid_file_name(const string file_name) {
        const string lowercased_file_name = change_case(file_name, TextCase::LOWER_CASE);

        if (lowercased_file_name.starts_with("headers/") && lowercased_file_name.length() == string("headers/").length()) {
            return std::make_tuple(false, "Full file name not provided");
        } else if (lowercased_file_name.starts_with("src/") || lowercased_file_name.starts_with("test/")) {
            return std::make_tuple(false, "File name cannot start with 'src/' or 'test/'");
        } else if (lowercased_file_name.starts_with("/") || lowercased_file_name.ends_with("/")) {
            return std::make_tuple(false, "File name cannot start or end with a slash, i.e. '/'");
        } else if (lowercased_file_name.ends_with(".cpp") || lowercased_file_name.ends_with(".hpp")) {
            return std::make_tuple(false, "File name cannot end with any extension, i.e. '.cpp' or '.hpp'");
        } else if (lowercased_file_name.ends_with("main")) {
            return std::make_tuple(false, "File cannot be named as 'main'");
        } else if (does_name_contain_special_characters(lowercased_file_name)) {
            return std::make_tuple(false, "File name can only contain alphanumeric characters and the underscore character");
        } else {
            return std::make_tuple(true, "");
        }
    }

    std::tuple<string, string, string> get_qualified_names(const string full_file_path) {
        string stemmed_name{""};
        string guard_name{""};
        string namespace_name{""};

        const int literal_length_of_headers = string("headers/").length();
        const int literal_length_of_src = string("src/").length();
        const int literal_length_of_extension = string(".cpp").length();

        if (full_file_path.starts_with("headers/")) {
            stemmed_name = full_file_path.substr(literal_length_of_headers, full_file_path.length() - (literal_length_of_headers + literal_length_of_extension));
        } else if (full_file_path.starts_with("src/")) {
            stemmed_name = full_file_path.substr(literal_length_of_src, full_file_path.length() - (literal_length_of_src + literal_length_of_extension));
        }

        return std::make_tuple(
            stemmed_name,
            convert_stemmed_name_to_guard_name(stemmed_name),
            convert_stemmed_name_to_namespace_name(stemmed_name)
        );
    }

    string convert_stemmed_name_to_guard_name(const string stemmed_name) {
        return std::regex_replace(
            change_case(stemmed_name, TextCase::UPPER_CASE),
            std::regex("/", std::regex_constants::basic),
            string("_")
        );
    }

    string convert_stemmed_name_to_namespace_name(const string stemmed_name) {
        return std::regex_replace(
            change_case(stemmed_name, TextCase::LOWER_CASE),
            std::regex("/", std::regex_constants::basic),
            string("::")
        );
    }

    std::tuple<string, string> get_key_value_pair_from_line(const string line, const string delimiter) {
        const int delimiter_position = line.find(delimiter);

        const string key = line.substr(0, delimiter_position);
        const string value = line.substr(delimiter_position + 1);

        return std::make_tuple(key, value);
    }
}
