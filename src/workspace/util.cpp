#include "workspace/util.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
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

    std::tuple<bool, string> does_name_contain_special_characters(const string& text, const bool is_it_for_project) {
        int i{ 0 };

        for (const char ch: text) {
            const auto uch = static_cast<unsigned char>(ch);

            if (i == 0 && (uch >= '0' && uch <='9')) {
                return std::make_tuple(true, string(is_it_for_project ? "Project" : "File") + " name cannot begin with a digit");
            } else if (std::isspace(uch) || std::isblank(uch)) {
                return std::make_tuple(true, string(is_it_for_project ? "Project" : "File") + " name cannot contain space or blank characters");
            } else if (is_it_for_project) {
                // disallow '/'; if necessary, traverse into the path and invoke
                if (std::ispunct(uch) && !std::isalnum(uch) && uch != '_' && uch != '-') {
                    return std::make_tuple(true, "Project name cannot contain punctuation symbols except '_' and '-'");
                } 
            } else {
                if (std::ispunct(uch) && !std::isalnum(uch) && uch != '_' && uch != '/') {
                    return std::make_tuple(true, "File name cannot contain punctuation symbols except '_' and '/'");
                } else if (i > 0 && text[i-1] == '/' && (uch >= '0' && uch <='9')) {
                    return std::make_tuple(true, "File/Directory name cannot start with a digit");
                }
            }

            ++i;
        }

        return std::make_tuple(false, "");
    }

    std::tuple<bool, string> is_valid_project_name(const string project_name) {
        const string lowercased_project_name = change_case(project_name, TextCase::LOWER_CASE);

        if (lowercased_project_name.find('/') != lowercased_project_name.npos || lowercased_project_name.find('\\') != lowercased_project_name.npos) {
            return std::make_tuple(false, "Project name cannot contain '/' or '\\'");
        } else {
            const auto [is_invalid, reason_if_any] = does_name_contain_special_characters(lowercased_project_name, true);
            
            if (is_invalid) {
                return std::make_tuple(false, reason_if_any);
            } else {
                return std::make_tuple(true, "");
            }
        }
    }
    
    std::tuple<bool, string> is_valid_file_name(const string file_name) {
        const string lowercased_file_name = change_case(file_name, TextCase::LOWER_CASE);

        if (lowercased_file_name.starts_with("headers/") && lowercased_file_name.length() == string("headers/").length()) {
            return std::make_tuple(false, "Full file name not provided");
        } else if (lowercased_file_name.starts_with("src/") || lowercased_file_name.starts_with("tests/")  || lowercased_file_name.starts_with("tests/unit_tests")) {
            return std::make_tuple(false, "File name cannot start with 'src/' or 'tests/' or 'tests/unit_tests/'");
        } else if (lowercased_file_name.starts_with("/") || lowercased_file_name.ends_with("/")) {
            return std::make_tuple(false, "File name cannot start or end with a slash, i.e. '/'");
        } else if (lowercased_file_name.ends_with(".cpp") || lowercased_file_name.ends_with(".hpp")) {
            return std::make_tuple(false, "File name cannot end with any extension, i.e. '.cpp' or '.hpp'");
        } else if (lowercased_file_name.ends_with("main")) {
            return std::make_tuple(false, "File cannot be named as 'main'");
        } else {
            const auto [is_invalid, reason_if_any] = does_name_contain_special_characters(lowercased_file_name, false);

            if (is_invalid) {
                return std::make_tuple(false, reason_if_any);
            } else {
                return std::make_tuple(true, "");
            }
        }
    }

    std::tuple<string, string, string> get_qualified_names(const string full_file_path) {
        string stemmed_name{""};

        const int literal_length_of_headers = string("headers/").length();
        const int literal_length_of_src = string("src/").length();
        const int literal_length_of_tests = string("tests/unit_tests/").length();
        const int literal_length_of_extension = string(".cpp").length();

        if (full_file_path.starts_with("headers/")) {
            stemmed_name = full_file_path.substr(literal_length_of_headers, full_file_path.length() - (literal_length_of_headers + literal_length_of_extension));
        } else if (full_file_path.starts_with("src/")) {
            stemmed_name = full_file_path.substr(literal_length_of_src, full_file_path.length() - (literal_length_of_src + literal_length_of_extension));
        } else if (full_file_path.starts_with("tests/unit_tests/")) {
            stemmed_name = full_file_path.substr(literal_length_of_tests, full_file_path.length() - (literal_length_of_tests + literal_length_of_extension));
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

    string get_platform_formatted_filename(const string file_name) {
        return std::filesystem::path(file_name).make_preferred().string();
    }

    string get_platform_formatted_filename(std::filesystem::path path) {
        return path.make_preferred().string();
    }

    string get_ISO_date() {
        const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()) };

        const string yyyy = std::to_string(int(ymd.year()));

        const unsigned month = static_cast<unsigned>(ymd.month());
        const string mm = (month < 10 ? "0" : "") + std::to_string(month);

        const unsigned day = static_cast<unsigned>(ymd.day());
        const string dd = (day < 10 ? "0" : "") + std::to_string(day);

        return yyyy + "-" + mm + "-" + dd;
    }
}
