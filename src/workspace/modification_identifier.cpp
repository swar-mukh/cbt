#include "workspace/modification_identifier.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "workspace/project_config.hpp"
#include "workspace/util.hpp"

namespace {
    using namespace workspace::modification_identifier;

    namespace cr = std::chrono;
    namespace fs = std::filesystem;

    using std::string;

    using DB = std::map<FileHash, SourceFile>;
    using RawDependencyTree = std::map<string, std::vector<string>>;

    const string MAKEFILE_PATH{ ".internals/tmp/makefile" };
    const string TIMESTAMPS_PATH{ ".internals/timestamps.txt" };
    
    std::tuple<FileHash, fs::path> compute_hash_and_file_pair(const string& file_name) {
        const string normalised_file_name = workspace::util::get_platform_formatted_filename(file_name);
        const fs::path normalised_path = fs::path(normalised_file_name);

        return std::make_tuple(fs::hash_value(normalised_file_name), normalised_path);
    }

    std::size_t get_last_modified_timestamp(const fs::path& path) {
        return static_cast<std::size_t>(cr::duration_cast<cr::seconds>(fs::last_write_time(path).time_since_epoch()).count());
    }

    std::tuple<FileHash, SourceFile> parse_line(const string& line) {
        std::stringstream stream(line);

        FileHash hash;
        stream >> hash;

        stream.ignore(1);

        std::size_t last_modified_timestamp;
        stream >> last_modified_timestamp;

        stream.ignore(1);
        
        std::size_t compilation_start_timestamp;
        stream >> compilation_start_timestamp;

        stream.ignore(1);
        
        std::size_t compilation_end_timestamp;
        stream >> compilation_end_timestamp;

        stream.ignore(1);

        bool was_successful;
        stream >> was_successful;

        stream.ignore(1);

        return std::make_tuple(hash, SourceFile{
            .hash{ hash },
            .file_name{},
            .last_modified_timestamp{ last_modified_timestamp },
            .compilation_start_timestamp{ compilation_start_timestamp },
            .compilation_end_timestamp{ compilation_end_timestamp },
            .affected { false },
            .was_successful{ was_successful }
        });
    }

    DB read_internal_timestamps_file() {
        const string timestamps_file_name{ ".internals/timestamps.txt" };
        DB files_with_timestamps{};

        if (fs::exists(timestamps_file_name)) {
            std::ifstream timestamps_file(timestamps_file_name);
            string line;

            while (std::getline(timestamps_file, line)) {
                std::erase(line, '\r');

                if (line.empty() || line.starts_with(";")) {
                    continue;
                }

                const auto [hash, source_file] = parse_line(line);
                files_with_timestamps[hash] = source_file;
            }
        } else {
            throw std::runtime_error("Timestamps file '.internals/timestamps.txt' missing!");
        }

        return files_with_timestamps;
    }

    void generate_makefile(const workspace::project_config::Project& project) {
        string files{ "src/*.cpp " };
        const string SEPARATOR{ fs::path::preferred_separator };

        for (auto dir_entry = fs::recursive_directory_iterator("src"); dir_entry != fs::recursive_directory_iterator(); ++dir_entry) {
            const string normalised_path{ workspace::util::get_platform_formatted_filename(dir_entry->path().string()) };

            if (fs::is_directory(*dir_entry)) {
                const int files_count = std::count_if(
                    fs::directory_iterator(dir_entry->path()),
                    {}, 
                    [](auto& file){ return file.is_regular_file(); }
                );

                if (files_count != 0) {
                    files += normalised_path + SEPARATOR + "*.cpp ";
                }
            }
        }

        if (fs::exists(MAKEFILE_PATH)) {
            fs::remove(MAKEFILE_PATH);
        }

        const int result = system((string("g++") + " -std=" + project.config.cpp_standard + " -Iheaders/ -MM " + files + " >> .internals/tmp/makefile").c_str());

        if (result != 0) {
            throw std::runtime_error("Could not compile project!");
        }
    }

    RawDependencyTree parse_makefile() {
        const string makefile_name{ MAKEFILE_PATH };

        if (fs::exists(makefile_name)) {
            std::ifstream makefile(makefile_name);
            string line;

            RawDependencyTree cpp_pov;

            bool keep_continuing{ false };
            string current_file{""};
            const string DELIMITER{ ": " };

            #if defined(_WIN32) || defined(_WIN64)
            const std::regex pattern("([\\w\\/\\.\\-\\\\]+)");
            #else
            const std::regex pattern("([\\w\\/\\.\\-]+)");
            #endif

            while (std::getline(makefile, line)) {
                std::erase(line, '\r');

                if (line.empty()) {
                    continue;
                }

                const auto start_index{ line.find(DELIMITER) };

                if (start_index != line.npos) {
                    line = line.substr(start_index + 2);
                }

                if (line.ends_with("\\")) {
                    keep_continuing = true;
                } else {
                    keep_continuing = false;
                }

                const std::sregex_iterator words = std::sregex_iterator(line.begin(), line.end(), pattern);

                for (std::sregex_iterator match = words; match != std::sregex_iterator(); ++match) {
                    string file = match->str();

                    if (current_file.empty()) {
                        current_file = file;

                        cpp_pov[current_file] = {};
                    } else {
                        cpp_pov[current_file].push_back(file);
                    }
                }

                if (!keep_continuing) {
                    current_file.clear();
                }
            }

            return cpp_pov;
        } else {
            throw std::runtime_error("Makefile '.internals/tmp/makefile' missing!");
        }
    }

    RawDependencyTree convert_to_hpp_pov(const RawDependencyTree& cpp_pov) {
        RawDependencyTree hpp_pov;

        for (auto const& [file, dependencies]: cpp_pov) {
            for (auto const& dependency: dependencies) {
                if (!hpp_pov.contains(dependency)) {
                    hpp_pov[dependency] = { file };
                } else {
                    hpp_pov[dependency].push_back(file);
                }
            }
        }

        return hpp_pov;
    }
   
    SourceFile get_or_construct_source_file(const string& file_name, DB& timestamps_history) {
        const auto [hash, file_path] = compute_hash_and_file_pair(file_name);

        if (timestamps_history.contains(hash)) {
            SourceFile source_file = timestamps_history[hash];

            const FileHash file_last_modified_timestamp = get_last_modified_timestamp(file_path);

            source_file.file_name = workspace::util::get_platform_formatted_filename(file_path);
            
            if ((file_last_modified_timestamp > source_file.last_modified_timestamp)
                || (source_file.file_name.ends_with(".cpp")
                    && (!source_file.was_successful || file_last_modified_timestamp > source_file.compilation_end_timestamp))
            ) {
                source_file.affected = true;
            }
            
            source_file.last_modified_timestamp = file_last_modified_timestamp;

            return source_file;
        } else {
            return SourceFile{
                .hash{ hash },
                .file_name{ workspace::util::get_platform_formatted_filename(file_path) },
                .last_modified_timestamp{ get_last_modified_timestamp(file_path) },
                .compilation_start_timestamp{ static_cast<std::size_t>(0) },
                .compilation_end_timestamp{ static_cast<std::size_t>(0) },
                .affected { true },
                .was_successful{ false }
            };
        }
    }

    SourceFiles construct_annotated_list_of_source_files(
        RawDependencyTree& hpp_pov,
        DB& timestamps_history
    ) {
        SourceFiles bucket{};
        
        for (auto& [header_file, dependants]: hpp_pov) {
            SourceFile hpp_file{ get_or_construct_source_file(header_file, timestamps_history) };
            bucket.insert(hpp_file);

            for (auto& dependant: dependants) {
                SourceFile cpp_file{ get_or_construct_source_file(dependant, timestamps_history) };

                if (hpp_file.affected) {
                    cpp_file.affected = true;
                }

                if (!bucket.contains(cpp_file)) {
                    bucket.insert(cpp_file);
                } else {
                    if (auto existing_file = bucket.find(cpp_file); existing_file != bucket.end()) {
                        if (!existing_file->affected && hpp_file.affected) {
                            existing_file->affected = true;
                        }
                    }
                }
            }
        }

        return bucket;
    }
}

namespace workspace::modification_identifier {
    std::size_t get_current_fileclock_timestamp() {
        return static_cast<std::size_t>(cr::duration_cast<cr::seconds>(cr::file_clock::now().time_since_epoch()).count());
    }

    SourceFiles list_all_files_annotated(const workspace::project_config::Project& project) {
        DB timestamps_history = read_internal_timestamps_file();

        generate_makefile(project);

        RawDependencyTree cpp_pov = parse_makefile();
        RawDependencyTree hpp_pov = convert_to_hpp_pov(cpp_pov);

        SourceFiles bucket = construct_annotated_list_of_source_files(hpp_pov, timestamps_history);

        fs::remove(MAKEFILE_PATH);

        return bucket;
    }

    void persist_annotations(const SourceFiles& bucket) {
        if (fs::exists(TIMESTAMPS_PATH)) {
            fs::remove(TIMESTAMPS_PATH);
        }

        std::ofstream file_to_write(TIMESTAMPS_PATH);

        file_to_write << "; CBT generated. DO NOT EDIT!\n";

        for(auto const& source_file: bucket) {
            file_to_write << source_file.hash
                << "," << source_file.last_modified_timestamp
                << "," << source_file.compilation_start_timestamp
                << "," << source_file.compilation_end_timestamp
                << "," << source_file.was_successful << "\n";
        }
        
        file_to_write.close();
    }
}
