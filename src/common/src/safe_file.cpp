#include "common/safe_file.h"
#include "common/logger.h"
#include "common/misc.h"
#include <boost/algorithm/string.hpp>
#include <ranges>
#include <stack>

namespace common {

namespace details {

constexpr auto TEMP_FILE_SUFFIX = ".~temp~";

static std::filesystem::path temp_file(const std::filesystem::path& filename)
{
    return filename.string() + TEMP_FILE_SUFFIX;
}
} // namespace details
safe_file::safe_file(
    const std::filesystem::path& filename,
    std::ios_base::openmode
        _Mode /*= std::ios_base::out | std::ios_base::binary | std::ios_base::trunc*/)
    : temp_filename_(details::temp_file(filename))
    , filename_(filename)
{
    std::error_code ec;
    std::filesystem::create_directories(filename.parent_path(), ec);

    if (std::filesystem::exists(filename_, ec)) {
        std::filesystem::copy_file(
            filename_, temp_filename_, std::filesystem::copy_options::overwrite_existing, ec);
    }
    temp_file_.open(temp_filename_, _Mode);
    if (!temp_file_) {
        common::logger::warn("打开临时文件失败: {}", misc::to_u8string(temp_filename_.string()));
    }
}

safe_file::~safe_file()
{
    temp_file_.close();
    move_file(temp_filename_, filename_);
}

std::ofstream& safe_file::get()
{
    return temp_file_;
}

void safe_file::copy_directory(const std::filesystem::path& source,
                               const std::filesystem::path& destination,
                               bool skip_exists)
{
    std::error_code ec;
    for (const auto& file_entry : std::filesystem::directory_iterator(source, ec)) {
        const auto& src_path = file_entry.path();
        const auto& dst_path = destination / src_path.filename();

        if (src_path.extension() == details::TEMP_FILE_SUFFIX)
            continue;

        if (std::filesystem::is_directory(src_path, ec))
            copy_directory(src_path, dst_path);
        else if (std::filesystem::is_regular_file(src_path, ec))
            copy_file(src_path, dst_path, skip_exists);
    }
    if (ec) {
        common::logger::warn("拷贝文件夹:{} 遍历异常:{}",
                             misc::to_u8string(source.string()),
                             misc::to_u8string(ec.message()));
    }
}

void safe_file::move_directory(const std::filesystem::path& source,
                               const std::filesystem::path& destination)
{
    std::error_code ec;
    for (const auto& file_entry : std::filesystem::directory_iterator(source, ec)) {
        const auto& src_path = file_entry.path();
        const auto& dst_path = destination / src_path.filename();

        if (src_path.extension() == details::TEMP_FILE_SUFFIX)
            continue;

        if (std::filesystem::is_directory(src_path, ec))
            move_directory(src_path, dst_path);
        else if (std::filesystem::is_regular_file(src_path, ec))
            move_file(src_path, dst_path);
    }
    if (ec) {
        common::logger::warn("移动文件夹:{} 遍历异常:{}",
                             misc::to_u8string(source.string()),
                             misc::to_u8string(ec.message()));
    }
    else {
        std::filesystem::remove(source, ec);
        if (ec) {
            common::logger::warn("删除源文件夹:{} 异常:{}",
                                 misc::to_u8string(source.string()),
                                 misc::to_u8string(ec.message()));
        }
    }
}

void safe_file::remove_directory(const std::filesystem::path& source)
{
    std::error_code ec;
    for (const auto& file_entry : std::filesystem::directory_iterator(source, ec)) {
        const auto& src_path = file_entry.path();

        if (std::filesystem::is_directory(src_path, ec))
            remove_directory(src_path);
        else if (std::filesystem::is_regular_file(src_path, ec))
            remove_file(src_path);
    }
    if (ec) {
        common::logger::warn("删除文件夹:{} 遍历异常:{}",
                             misc::to_u8string(source),
                             misc::to_u8string(ec.message()));
    }
    else {
        std::filesystem::remove(source, ec);
        if (ec) {
            common::logger::warn("删除源文件夹:{} 异常:{}",
                                 misc::to_u8string(source),
                                 misc::to_u8string(ec.message()));
        }
    }
}

bool safe_file::copy_file_to_dir(const std::filesystem::path& source,
                                 const std::filesystem::path& destination,
                                 bool skip_exists)
{
    if (!exists(source)) {
        common::logger::warn("拷贝文件源文件不存在: {}", misc::to_u8string(source.string()));
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(source, ec)) {
        common::logger::warn("拷贝文件源文件不是文件: {}", misc::to_u8string(source.string()));
        return false;
    }

    if (source.parent_path() == destination) {
        common::logger::warn("拷贝文件目标文件夹和源文件是同一个: {} 目标文件夹: {}",
                             misc::to_u8string(source.string()),
                             misc::to_u8string(destination.string()));
        return false;
    }
    auto dest_file_path = destination / source.filename();
    return safe_file::copy_file(source, dest_file_path, skip_exists);
}

bool safe_file::move_file_to_dir(const std::filesystem::path& source,
                                 const std::filesystem::path& destination)
{
    return move_file(source, destination / source.filename());
}

bool safe_file::copy_file(const std::filesystem::path& source,
                          const std::filesystem::path& destination,
                          bool skip_exists)
{
    if (!exists(source))
        return false;

    std::error_code ec;
    if (skip_exists && std::filesystem::exists(destination, ec))
        return true;

    auto dest_dir = destination.parent_path();

    if (!std::filesystem::exists(dest_dir, ec) || !std::filesystem::is_directory(dest_dir, ec))
        std::filesystem::create_directories(dest_dir, ec);


    auto dest_file_temp_path = details::temp_file(destination);

    std::filesystem::copy_file(
        source, dest_file_temp_path, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
        common::logger::warn("拷贝到临时文件失败: {} what:{}",
                             misc::to_u8string(dest_file_temp_path),
                             misc::to_u8string(ec.message()));
        return false;
    }
    if (std::filesystem::exists(destination, ec)) {
        common::logger::trace("拷贝目标文件已存在: {} 会先删除再拷贝",
                              misc::to_u8string(destination)); 
        std::filesystem::remove(destination, ec);
        if (ec) {
            common::logger::warn("拷贝时删除已存在的目标文件: {} 失败, what: {}",
                                 misc::to_u8string(destination),
                                 misc::to_u8string(ec.message()));
        }
    }

    std::filesystem::rename(dest_file_temp_path, destination, ec);
    if (ec) {
        common::logger::warn("拷贝文件时重命名文件: {} 到 {} 失败，what: {}",
                             misc::to_u8string(source),
                             misc::to_u8string(destination),
                             misc::to_u8string(ec.message()));
        std::filesystem::remove(dest_file_temp_path, ec);
        return false;
    }
    common::logger::trace("拷贝文件: {} 到 {} 成功",
                          misc::to_u8string(source.string()),
                          misc::to_u8string(destination.string()));
    return true;
}

bool safe_file::move_file(const std::filesystem::path& source,
                          const std::filesystem::path& destination)
{
    if (!exists(source))
        return false;

    do {
        auto dest_dir = destination.parent_path();
        std::error_code ec;
        if (!std::filesystem::exists(dest_dir, ec) || !std::filesystem::is_directory(dest_dir, ec))
            std::filesystem::create_directories(dest_dir, ec);

        std::filesystem::rename(source, destination, ec);
        if (ec)
            break;

        common::logger::trace("移动文件: {} 到 {} 成功",
                              misc::to_u8string(source.string()),
                              misc::to_u8string(destination.string()));
        return true;
    }
    while (false);

    if (!copy_file(source, destination))
        return false;
    remove_file(source);
    return true;
}

bool safe_file::remove_file(const std::filesystem::path& source)
{
    std::error_code ec;
    std::filesystem::remove(source, ec);
    if (ec) {
        common::logger::warn(R"(请注意删除文件: {} 失败, what: {})",
                             misc::to_u8string(source.string()),
                             misc::to_u8string(ec.message()));
        return false;
    }
    common::logger::trace(R"(删除文件: {} 成功)", misc::to_u8string(source.string()));
    return true;
}

bool safe_file::exists(const std::filesystem::path& source)
{
    std::error_code ec;
    bool exist = std::filesystem::exists(source, ec);
    if (ec) {
        common::logger::warn(R"(判断路径是否存在: {} 异常: {})",
                             misc::to_u8string(source.string()),
                             misc::to_u8string(ec.message()));
    }
    return exist;
}

std::generator<std::filesystem::path> safe_file::walk_files(std::filesystem::path dir,
                                                            std::set<std::string> exts /*= {}*/,
                                                            bool recursive /*= false*/)
{
    std::stack<std::filesystem::path> _dirs;
    _dirs.push(std::move(dir));

    while (!_dirs.empty()) {
        auto current_dir = std::move(_dirs.top());
        _dirs.pop();

        std::error_code ec;
        for (const auto& file_entry : std::filesystem::directory_iterator(current_dir, ec)) {
            if (file_entry.is_directory(ec)) {
                if (recursive)
                    _dirs.push(file_entry.path());
                continue;
            }
            if (!file_entry.is_regular_file(ec))
                continue;

            const auto& p = file_entry.path();
            if (exts.empty())
                co_yield p;

            if (!p.has_extension())
                continue;

            auto file_ext = p.extension().string();

            auto iter = std::ranges::find_if(
                exts, [&](const auto& ext) { return boost::iequals(file_ext, ext); });
            if (iter == exts.end())
                continue;

            co_yield p;
        }
        if (ec)
            common::logger::warn("获取目录:{} 下的文件异常: {}",
                                 misc::to_u8string(current_dir.string()),
                                 misc::to_u8string(ec.message()));
    }
}

std::vector<std::string> safe_file::read_lines(const std::filesystem::path& source,
                                               bool skip_empty /*= true*/)
{
    std::vector<std::string> lines;

    std::string line;
    std::ifstream file(source);
    if (!file) {
        common::logger::warn("无法打开文件:{}", misc::to_u8string(source));
        return lines;
    }
    while (std::getline(file, line)) {
        if (line.empty() && skip_empty)
            continue;
        lines.push_back(line);
    }
    return lines;
}

std::string safe_file::read_all(const std::filesystem::path& source)
{
    if (!exists(source))
        return {};

    std::error_code ec;
    if (!std::filesystem::is_regular_file(source, ec)) {
        common::logger::warn("不是常规文件: {}", misc::to_u8string(source));
        return {};
    }
    auto file_size = std::filesystem::file_size(source, ec);
    if (file_size == 0) {
        return {};
    }

    std::ifstream file(source, std::ios::binary);
    if (!file.is_open()) {
        common::logger::warn("无法打开文件: {}", misc::to_u8string(source));
        return {};
    }
    std::string content;
    content.reserve(file_size);
    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return content;
}

} // namespace common
