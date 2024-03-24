#ifndef WORKSPACE_UTIL
#define WORKSPACE_UTIL

#include <string>
#include <tuple>

namespace workspace::util {
    using std::string;

    enum class TextCase {
        LOWER_CASE,
        UPPER_CASE
    };

    string change_case(string text, const TextCase casing);
    bool does_name_contain_special_characters(const string& text);
    std::tuple<bool, string> is_valid_file_name(const string file_name);
    std::tuple<string, string, string> get_qualified_names(const string full_file_path);
    string convert_stemmed_name_to_guard_name(const string stemmed_name);
    string convert_stemmed_name_to_namespace_name(const string stemmed_name);
    std::tuple<string, string> get_key_value_pair_from_line(const string line, const string delimiter);
}

#endif