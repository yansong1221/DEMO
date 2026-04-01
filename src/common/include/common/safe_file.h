#pragma once
#include <filesystem>
#include <fstream>
#include <generator>
#include <list>
#include <optional>
#include <ranges>
#include <set>

namespace common {
class safe_file
{
public:
    safe_file(const std::filesystem::path& filename,
              std::ios_base::openmode _Mode = std::ios_base::out | std::ios_base::binary |
                                              std::ios_base::trunc);
    ~safe_file();

public:
    std::ofstream& get();
    operator std::ofstream&() { return temp_file_; }
    std::ofstream& operator*() { return temp_file_; }
    std::ofstream* operator->() { return &temp_file_; }

public:
    static void copy_directory(const std::filesystem::path& source,
                               const std::filesystem::path& destination,
                               bool skip_exists = false);
    static void move_directory(const std::filesystem::path& source,
                               const std::filesystem::path& destination);
    static void remove_directory(const std::filesystem::path& source);

    static bool copy_file_to_dir(const std::filesystem::path& source,
                                 const std::filesystem::path& destination,
                                 bool skip_exists = false);
    static bool move_file_to_dir(const std::filesystem::path& source,
                                 const std::filesystem::path& destination);

    static bool copy_file(const std::filesystem::path& source,
                          const std::filesystem::path& destination,
                          bool skip_exists = false);
    static bool move_file(const std::filesystem::path& source,
                          const std::filesystem::path& destination);

    static bool remove_file(const std::filesystem::path& source);

    static bool exists(const std::filesystem::path& source);

    inline static auto collect_walk_files(const std::filesystem::path& dir,
                                          const std::set<std::string>& exts = {},
                                          bool recursive                    = false)
    {
        return walk_files(dir, exts, recursive) | std::ranges::to<std::vector>();
    }

    static std::generator<std::filesystem::path> walk_files(std::filesystem::path dir,
                                                            std::set<std::string> exts = {},
                                                            bool recursive             = false);

    static std::vector<std::string> read_lines(const std::filesystem::path& source,
                                               bool skip_empty = true);
    static std::string read_all(const std::filesystem::path& source);

private:
    std::ofstream temp_file_;
    std::filesystem::path filename_;
    std::filesystem::path temp_filename_;
};
} // namespace common