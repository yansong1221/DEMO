#pragma once
#include <filesystem>
#include <fstream>
#include <generator>
#include <list>
#include <optional>
#include <ranges>
#include <set>

namespace common
{
    class SafeFile
    {
      public:
        SafeFile(std::filesystem::path const& filename,
                 std::ios_base::openmode _Mode = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        ~SafeFile();

      public:
        std::ofstream& get();
        operator std::ofstream&() { return temp_file_; }
        std::ofstream&
        operator*()
        {
            return temp_file_;
        }
        std::ofstream*
        operator->()
        {
            return &temp_file_;
        }

      public:
        static void copyDirectory(std::filesystem::path const& source,
                                  std::filesystem::path const& destination,
                                  bool skip_exists = false);
        static void moveDirectory(std::filesystem::path const& source, std::filesystem::path const& destination);
        static void removeDirectory(std::filesystem::path const& source);

        static bool copyFileToDirectory(std::filesystem::path const& source,
                                        std::filesystem::path const& destination,
                                        bool skip_exists = false);
        static bool moveFileToDirectory(std::filesystem::path const& source, std::filesystem::path const& destination);

        static bool copyFile(std::filesystem::path const& source,
                             std::filesystem::path const& destination,
                             bool skip_exists = false);
        static bool moveFile(std::filesystem::path const& source, std::filesystem::path const& destination);

        static bool removeFile(std::filesystem::path const& source);

        static bool exists(std::filesystem::path const& source);

        inline static auto
        collectWalkFiles(std::filesystem::path const& dir,
                         std::set<std::string> const& exts = {},
                         bool recursive = false)
        {
            return walkFiles(dir, exts, recursive) | std::ranges::to<std::vector>();
        }

        static std::generator<std::filesystem::path> walkFiles(std::filesystem::path dir,
                                                               std::set<std::string> exts = {},
                                                               bool recursive = false);

        static std::vector<std::string> readLines(std::filesystem::path const& source, bool skip_empty = true);
        static std::string readAll(std::filesystem::path const& source);

      private:
        std::ofstream temp_file_;
        std::filesystem::path filename_;
        std::filesystem::path temp_filename_;
    };
} // namespace common