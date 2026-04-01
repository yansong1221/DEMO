#include "common/SafeFile.h"
#include "common/logger.h"
#include "common/misc.h"
#include <boost/algorithm/string.hpp>
#include <ranges>
#include <stack>

namespace common
{

    namespace details
    {

        constexpr auto TEMP_FILE_SUFFIX = ".~temp~";

        static std::filesystem::path
        temp_file(std::filesystem::path const& filename)
        {
            return filename.string() + TEMP_FILE_SUFFIX;
        }
    } // namespace details
    SafeFile::SafeFile(
        std::filesystem::path const& filename,
        std::ios_base::openmode _Mode /*= std::ios_base::out | std::ios_base::binary | std::ios_base::trunc*/)
        : temp_filename_(details::temp_file(filename))
        , filename_(filename)
    {
        std::error_code ec;
        std::filesystem::create_directories(filename.parent_path(), ec);

        if (std::filesystem::exists(filename_, ec))
        {
            std::filesystem::copy_file(filename_,
                                       temp_filename_,
                                       std::filesystem::copy_options::overwrite_existing,
                                       ec);
        }
        temp_file_.open(temp_filename_, _Mode);
        if (!temp_file_)
        {
            common::Log::warn("打开临时文件失败: {}", misc::to_u8string(temp_filename_.string()));
        }
    }

    SafeFile::~SafeFile()
    {
        temp_file_.close();
        moveFile(temp_filename_, filename_);
    }

    std::ofstream&
    SafeFile::get()
    {
        return temp_file_;
    }

    void
    SafeFile::copyDirectory(std::filesystem::path const& source,
                            std::filesystem::path const& destination,
                            bool skip_exists)
    {
        std::error_code ec;
        for (auto const& file_entry : std::filesystem::directory_iterator(source, ec))
        {
            auto const& src_path = file_entry.path();
            auto const& dst_path = destination / src_path.filename();

            if (src_path.extension() == details::TEMP_FILE_SUFFIX)
            {
                continue;
            }

            if (std::filesystem::is_directory(src_path, ec))
            {
                copyDirectory(src_path, dst_path);
            }
            else if (std::filesystem::is_regular_file(src_path, ec))
            {
                copyFile(src_path, dst_path, skip_exists);
            }
        }
        if (ec)
        {
            common::Log::warn("拷贝文件夹:{} 遍历异常:{}",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(ec.message()));
        }
    }

    void
    SafeFile::moveDirectory(std::filesystem::path const& source, std::filesystem::path const& destination)
    {
        std::error_code ec;
        for (auto const& file_entry : std::filesystem::directory_iterator(source, ec))
        {
            auto const& src_path = file_entry.path();
            auto const& dst_path = destination / src_path.filename();

            if (src_path.extension() == details::TEMP_FILE_SUFFIX)
            {
                continue;
            }

            if (std::filesystem::is_directory(src_path, ec))
            {
                moveDirectory(src_path, dst_path);
            }
            else if (std::filesystem::is_regular_file(src_path, ec))
            {
                moveFile(src_path, dst_path);
            }
        }
        if (ec)
        {
            common::Log::warn("移动文件夹:{} 遍历异常:{}",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(ec.message()));
        }
        else
        {
            std::filesystem::remove(source, ec);
            if (ec)
            {
                common::Log::warn("删除源文件夹:{} 异常:{}",
                                  misc::to_u8string(source.string()),
                                  misc::to_u8string(ec.message()));
            }
        }
    }

    void
    SafeFile::removeDirectory(std::filesystem::path const& source)
    {
        std::error_code ec;
        for (auto const& file_entry : std::filesystem::directory_iterator(source, ec))
        {
            auto const& src_path = file_entry.path();

            if (std::filesystem::is_directory(src_path, ec))
            {
                removeDirectory(src_path);
            }
            else if (std::filesystem::is_regular_file(src_path, ec))
            {
                removeFile(src_path);
            }
        }
        if (ec)
        {
            common::Log::warn("删除文件夹:{} 遍历异常:{}", misc::to_u8string(source), misc::to_u8string(ec.message()));
        }
        else
        {
            std::filesystem::remove(source, ec);
            if (ec)
            {
                common::Log::warn("删除源文件夹:{} 异常:{}",
                                  misc::to_u8string(source),
                                  misc::to_u8string(ec.message()));
            }
        }
    }

    bool
    SafeFile::copyFileToDirectory(std::filesystem::path const& source,
                                  std::filesystem::path const& destination,
                                  bool skip_exists)
    {
        if (!exists(source))
        {
            common::Log::warn("拷贝文件源文件不存在: {}", misc::to_u8string(source.string()));
            return false;
        }

        std::error_code ec;
        if (!std::filesystem::is_regular_file(source, ec))
        {
            common::Log::warn("拷贝文件源文件不是文件: {}", misc::to_u8string(source.string()));
            return false;
        }

        if (source.parent_path() == destination)
        {
            common::Log::warn("拷贝文件目标文件夹和源文件是同一个: {} 目标文件夹: {}",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(destination.string()));
            return false;
        }
        auto dest_file_path = destination / source.filename();
        return SafeFile::copyFile(source, dest_file_path, skip_exists);
    }

    bool
    SafeFile::moveFileToDirectory(std::filesystem::path const& source, std::filesystem::path const& destination)
    {
        return moveFile(source, destination / source.filename());
    }

    bool
    SafeFile::copyFile(std::filesystem::path const& source, std::filesystem::path const& destination, bool skip_exists)
    {
        if (!exists(source))
        {
            return false;
        }

        std::error_code ec;
        if (skip_exists && std::filesystem::exists(destination, ec))
        {
            return true;
        }

        auto dest_dir = destination.parent_path();

        if (!std::filesystem::exists(dest_dir, ec) || !std::filesystem::is_directory(dest_dir, ec))
        {
            std::filesystem::create_directories(dest_dir, ec);
        }

        auto dest_file_temp_path = details::temp_file(destination);

        std::filesystem::copy_file(source, dest_file_temp_path, std::filesystem::copy_options::overwrite_existing, ec);
        if (ec)
        {
            common::Log::warn("拷贝到临时文件失败: {} what:{}",
                              misc::to_u8string(dest_file_temp_path),
                              misc::to_u8string(ec.message()));
            return false;
        }
        if (std::filesystem::exists(destination, ec))
        {
            common::Log::trace("拷贝目标文件已存在: {} 会先删除再拷贝", misc::to_u8string(destination));
            std::filesystem::remove(destination, ec);
            if (ec)
            {
                common::Log::warn("拷贝时删除已存在的目标文件: {} 失败, what: {}",
                                  misc::to_u8string(destination),
                                  misc::to_u8string(ec.message()));
            }
        }

        std::filesystem::rename(dest_file_temp_path, destination, ec);
        if (ec)
        {
            common::Log::warn("拷贝文件时重命名文件: {} 到 {} 失败，what: {}",
                              misc::to_u8string(source),
                              misc::to_u8string(destination),
                              misc::to_u8string(ec.message()));
            std::filesystem::remove(dest_file_temp_path, ec);
            return false;
        }
        common::Log::trace("拷贝文件: {} 到 {} 成功",
                           misc::to_u8string(source.string()),
                           misc::to_u8string(destination.string()));
        return true;
    }

    bool
    SafeFile::moveFile(std::filesystem::path const& source, std::filesystem::path const& destination)
    {
        if (!exists(source))
        {
            return false;
        }

        do
        {
            auto dest_dir = destination.parent_path();
            std::error_code ec;
            if (!std::filesystem::exists(dest_dir, ec) || !std::filesystem::is_directory(dest_dir, ec))
            {
                std::filesystem::create_directories(dest_dir, ec);
            }

            std::filesystem::rename(source, destination, ec);
            if (ec)
            {
                break;
            }

            common::Log::trace("移动文件: {} 到 {} 成功",
                               misc::to_u8string(source.string()),
                               misc::to_u8string(destination.string()));
            return true;
        } while (false);

        if (!copyFile(source, destination))
        {
            return false;
        }
        removeFile(source);
        return true;
    }

    bool
    SafeFile::removeFile(std::filesystem::path const& source)
    {
        std::error_code ec;
        std::filesystem::remove(source, ec);
        if (ec)
        {
            common::Log::warn(R"(请注意删除文件: {} 失败, what: {})",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(ec.message()));
            return false;
        }
        common::Log::trace(R"(删除文件: {} 成功)", misc::to_u8string(source.string()));
        return true;
    }

    bool
    SafeFile::exists(std::filesystem::path const& source)
    {
        std::error_code ec;
        bool exist = std::filesystem::exists(source, ec);
        if (ec)
        {
            common::Log::warn(R"(判断路径是否存在: {} 异常: {})",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(ec.message()));
        }
        return exist;
    }

    std::generator<std::filesystem::path>
    SafeFile::walkFiles(std::filesystem::path dir, std::set<std::string> exts /*= {}*/, bool recursive /*= false*/)
    {
        std::stack<std::filesystem::path> _dirs;
        _dirs.push(std::move(dir));

        while (!_dirs.empty())
        {
            auto current_dir = std::move(_dirs.top());
            _dirs.pop();

            std::error_code ec;
            for (auto const& file_entry : std::filesystem::directory_iterator(current_dir, ec))
            {
                if (file_entry.is_directory(ec))
                {
                    if (recursive)
                    {
                        _dirs.push(file_entry.path());
                    }
                    continue;
                }
                if (!file_entry.is_regular_file(ec))
                {
                    continue;
                }

                auto const& p = file_entry.path();
                if (exts.empty())
                {
                    co_yield p;
                }

                if (!p.has_extension())
                {
                    continue;
                }

                auto file_ext = p.extension().string();

                auto iter = std::ranges::find_if(exts, [&](auto const& ext) { return boost::iequals(file_ext, ext); });
                if (iter == exts.end())
                {
                    continue;
                }

                co_yield p;
            }
            if (ec)
            {
                common::Log::warn("获取目录:{} 下的文件异常: {}",
                                  misc::to_u8string(current_dir.string()),
                                  misc::to_u8string(ec.message()));
            }
        }
    }

    std::vector<std::string>
    SafeFile::readLines(std::filesystem::path const& source, bool skip_empty /*= true*/)
    {
        std::vector<std::string> lines;

        std::string line;
        std::ifstream file(source);
        if (!file)
        {
            common::Log::warn("无法打开文件:{}", misc::to_u8string(source));
            return lines;
        }
        while (std::getline(file, line))
        {
            if (line.empty() && skip_empty)
            {
                continue;
            }
            lines.push_back(line);
        }
        return lines;
    }

    std::string
    SafeFile::readAll(std::filesystem::path const& source)
    {
        if (!exists(source))
        {
            return {};
        }

        std::error_code ec;
        if (!std::filesystem::is_regular_file(source, ec))
        {
            common::Log::warn("不是常规文件: {}", misc::to_u8string(source));
            return {};
        }
        auto file_size = std::filesystem::file_size(source, ec);
        if (file_size == 0)
        {
            return {};
        }

        std::ifstream file(source, std::ios::binary);
        if (!file.is_open())
        {
            common::Log::warn("无法打开文件: {}", misc::to_u8string(source));
            return {};
        }
        std::string content;
        content.reserve(file_size);
        content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        return content;
    }

} // namespace common
